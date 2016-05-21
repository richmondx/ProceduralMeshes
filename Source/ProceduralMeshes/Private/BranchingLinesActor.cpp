// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example branching lines using cylinder strips

#include "ProceduralMeshesPrivatePCH.h"
#include "BranchingLinesActor.h"

ABranchingLinesActor::ABranchingLinesActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);

	// Setup random offset directions
	OffsetDirections.Add(FVector(1, 0, 0));
	OffsetDirections.Add(FVector(0, 0, 1));
}

#if WITH_EDITOR  
void ABranchingLinesActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	PreCacheCrossSection();
	GenerateMesh();
}
#endif // WITH_EDITOR

void ABranchingLinesActor::BeginPlay()
{
	Super::BeginPlay();
	PreCacheCrossSection();
	GenerateMesh();
}

void ABranchingLinesActor::GenerateMesh()
{
	// -------------------------------------------------------
	// Setup the random number generator and create the branching structure
	RngStream = FRandomStream::FRandomStream(RandomSeed);
	CreateSegments();

	// -------------------------------------------------------
	// Calculate and pre-allocate buffers
	int TotalNumberOfVerticesPerSection = RadialSegmentCount * 4; // 4 verts per face 
	int TotalNumberOfTrianglesPerSection = TotalNumberOfVerticesPerSection + 2 * RadialSegmentCount;

	FProceduralMeshData MeshData = FProceduralMeshData();
	MeshData.Vertices.AddUninitialized(TotalNumberOfVerticesPerSection * Segments.Num());
	MeshData.Triangles.AddUninitialized(TotalNumberOfTrianglesPerSection * Segments.Num());
	MeshData.Normals.AddUninitialized(TotalNumberOfVerticesPerSection * Segments.Num());
	MeshData.UVs.AddUninitialized(TotalNumberOfVerticesPerSection * Segments.Num());
	MeshData.Tangents.AddUninitialized(TotalNumberOfVerticesPerSection * Segments.Num());

	// -------------------------------------------------------
	// Now lets loop through all the defined segments and create a cylinder for each
	int CurrentIndexStart = 0;

	for (int i = 0; i < Segments.Num(); i++)
	{
		CurrentIndexStart = i * TotalNumberOfVerticesPerSection;
		GenerateCylinder(MeshData, Segments[i].Start, Segments[i].End, Segments[i].Width, RadialSegmentCount, CurrentIndexStart, bSmoothNormals);
	}

	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
	ProcMesh->SetMaterial(0, Material);
}

FVector ABranchingLinesActor::RotatePointAroundPivot(FVector InPoint, FVector InPivot, FVector InAngles)
{
	FVector direction = InPoint - InPivot; // get point direction relative to pivot
	direction = FQuat::MakeFromEuler(InAngles) * direction; // rotate it
	return direction + InPivot; // calculate rotated point
}

void ABranchingLinesActor::PreCacheCrossSection()
{
	if (LastCachedCrossSectionCount == RadialSegmentCount)
	{
		return;
	}

	// Generate a cross-section for use in cylinder generation
	const float AngleBetweenQuads = (2.0f / (float)(RadialSegmentCount)) * PI;
	CachedCrossSectionPoints.Empty();

	// Pre-calculate cross section points of a circle, two more than needed
	for (int32 PointIndex = 0; PointIndex < (RadialSegmentCount + 2); PointIndex++)
	{
		float Angle = (float)PointIndex * AngleBetweenQuads;
		CachedCrossSectionPoints.Add(FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0));
	}

	LastCachedCrossSectionCount = RadialSegmentCount;
}

void ABranchingLinesActor::CreateSegments()
{
	// We create the branching structure by constantly subdividing a line between two points by creating a new point in the middle.
	// We then take that point and offset it in a random direction, by a random amount defined within limits.
	// Next we take both of the newly created line halves, and subdivide them the same way.
	// Each new midpoint also has a chance to create a new branch
	// TODO This should really be recursive
	Segments.Empty();
	float CurrentBranchOffset = MaxBranchOffset;

	if (bMaxBranchOffsetAsPercentageOfLength)
	{
		CurrentBranchOffset = (Start - End).Size() * (FMath::Clamp(MaxBranchOffset, 0.1f, 100.0f) / 100.0f);
	}

	// Pre-calc a few floats from percentages
	float ChangeOfFork = FMath::Clamp(ChanceOfForkPercentage, 0.0f, 100.0f) / 100.0f;
	float BranchOffsetReductionEachGeneration = FMath::Clamp(BranchOffsetReductionEachGenerationPercentage, 0.0f, 100.0f) / 100.0f;

	// Add the first segment which is simply between the start and end points
	Segments.Add(FBranchSegment(Start, End, TrunkWidth));

	for (int iGen = 0; iGen < Iterations; iGen++)
	{
		TArray<FBranchSegment> newGen;

		for (const FBranchSegment& EachSegment : Segments)
		{
			FVector Midpoint = (EachSegment.End + EachSegment.Start) / 2;

			// Offset the midpoint by a random number along the normal
			FVector normal = FVector::CrossProduct(EachSegment.End - EachSegment.Start, OffsetDirections[RngStream.RandRange(0, 1)]);
			normal.Normalize();
			Midpoint += normal * RngStream.RandRange(-CurrentBranchOffset, CurrentBranchOffset);

			 // Create two new segments
			newGen.Add(FBranchSegment(EachSegment.Start, Midpoint, EachSegment.Width, EachSegment.ForkGeneration));
			newGen.Add(FBranchSegment(Midpoint, EachSegment.End, EachSegment.Width, EachSegment.ForkGeneration));

			// Chance of fork?
			if (RngStream.FRand() > (1 - ChangeOfFork))
			{
				// TODO Normalize the direction vector and calculate a new total length and then subdiv that for X generations
				FVector direction = Midpoint - EachSegment.Start;
				FVector splitEnd = (direction * RngStream.FRandRange(ForkLengthMin, ForkLengthMax)).RotateAngleAxis(RngStream.FRandRange(ForkRotationMin, ForkRotationMax), OffsetDirections[RngStream.RandRange(0, 1)]) + Midpoint;
				newGen.Add(FBranchSegment(Midpoint, splitEnd, EachSegment.Width * WidthReductionOnFork, EachSegment.ForkGeneration + 1));
			}
		}

		Segments.Empty();
		Segments = newGen;

		// Reduce the offset slightly each generation
		CurrentBranchOffset = CurrentBranchOffset * BranchOffsetReductionEachGeneration;
	}
}

void ABranchingLinesActor::GenerateCylinder(FProceduralMeshData& MeshData, FVector StartPoint, FVector EndPoint, float InWidth, int32 InCrossSectionCount, int InVertexIndexStart, bool bInSmoothNormals/* = true*/)
{
	// Basic setup
	int VertexIndex = InVertexIndexStart;
	int32 NumVerts = InCrossSectionCount * 4; // 4 verts per face

	// Make a cylinder section
	const float AngleBetweenQuads = (2.0f / (float)(InCrossSectionCount)) * PI;
	const float UMapPerQuad = 1.0f / (float)InCrossSectionCount;

	FVector StartOffset = StartPoint - FVector(0, 0, 0);
	FVector Offset = EndPoint - StartPoint;

	// Find angle between vectors
	FVector LineDirection = (StartPoint - EndPoint);
	LineDirection.Normalize();
	FVector RotationAngle = LineDirection.Rotation().Add(90.f, 0.f, 0.f).Euler();

	// Start by building up vertices that make up the cylinder sides
	for (int32 QuadIndex = 0; QuadIndex < InCrossSectionCount; QuadIndex++)
	{
		float Angle = (float)QuadIndex * AngleBetweenQuads;
		float NextAngle = (float)(QuadIndex + 1) * AngleBetweenQuads;

		// Set up the vertices
		FVector p0 = (CachedCrossSectionPoints[QuadIndex] * InWidth) + StartOffset;
		p0 = RotatePointAroundPivot(p0, StartPoint, RotationAngle);
		FVector p1 = CachedCrossSectionPoints[QuadIndex + 1] * InWidth + StartOffset;
		p1 = RotatePointAroundPivot(p1, StartPoint, RotationAngle);
		FVector p2 = p1 + Offset;
		FVector p3 = p0 + Offset;

		// Set up the quad triangles
		int VertIndex1 = VertexIndex++;
		int VertIndex2 = VertexIndex++;
		int VertIndex3 = VertexIndex++;
		int VertIndex4 = VertexIndex++;

		MeshData.Vertices[VertIndex1] = p0;
		MeshData.Vertices[VertIndex2] = p1;
		MeshData.Vertices[VertIndex3] = p2;
		MeshData.Vertices[VertIndex4] = p3;

		// Now create two triangles from those four vertices
		// The order of these (clockwise/counter-clockwise) dictates which way the normal will face. 
		MeshData.Triangles.Add(VertIndex4);
		MeshData.Triangles.Add(VertIndex3);
		MeshData.Triangles.Add(VertIndex1);

		MeshData.Triangles.Add(VertIndex3);
		MeshData.Triangles.Add(VertIndex2);
		MeshData.Triangles.Add(VertIndex1);

		// UVs.  Note that Unreal UV origin (0,0) is top left
		MeshData.UVs[VertIndex1] = FVector2D(1.0f - (UMapPerQuad * QuadIndex), 1.0f);
		MeshData.UVs[VertIndex2] = FVector2D(1.0f - (UMapPerQuad * (QuadIndex + 1)), 1.0f);
		MeshData.UVs[VertIndex3] = FVector2D(1.0f - (UMapPerQuad * (QuadIndex + 1)), 0.0f);
		MeshData.UVs[VertIndex4] = FVector2D(1.0f - (UMapPerQuad * QuadIndex), 0.0f);

		// Normals
		FVector NormalCurrent = FVector::CrossProduct(MeshData.Vertices[VertIndex1] - MeshData.Vertices[VertIndex3], MeshData.Vertices[VertIndex2] - MeshData.Vertices[VertIndex3]).GetSafeNormal();

		if (bInSmoothNormals)
		{
			// To smooth normals we give the vertices different values than the polygon they belong to.
			// GPUs know how to interpolate between those.
			// I do this here as an average between normals of two adjacent polygons
			float NextNextAngle = (float)(QuadIndex + 2) * AngleBetweenQuads;
			FVector p4 = (CachedCrossSectionPoints[QuadIndex + 2] * InWidth) + StartOffset;
			p4 = RotatePointAroundPivot(p4, StartPoint, RotationAngle);

			// p1 to p4 to p2
			FVector NormalNext = FVector::CrossProduct(p1 - p2, p4 - p2).GetSafeNormal();
			FVector AverageNormalRight = (NormalCurrent + NormalNext) / 2;
			AverageNormalRight = AverageNormalRight.GetSafeNormal();

			float PreviousAngle = (float)(QuadIndex - 1) * AngleBetweenQuads;
			FVector pMinus1 = FVector(FMath::Cos(PreviousAngle) * InWidth, FMath::Sin(PreviousAngle) * InWidth, 0.f) + StartOffset;
			pMinus1 = RotatePointAroundPivot(pMinus1, StartPoint, RotationAngle);

			// p0 to p3 to pMinus1
			FVector NormalPrevious = FVector::CrossProduct(p0 - pMinus1, p3 - pMinus1).GetSafeNormal();
			FVector AverageNormalLeft = (NormalCurrent + NormalPrevious) / 2;
			AverageNormalLeft = AverageNormalLeft.GetSafeNormal();

			MeshData.Normals[VertIndex1] = AverageNormalLeft;
			MeshData.Normals[VertIndex2] = AverageNormalRight;
			MeshData.Normals[VertIndex3] = AverageNormalRight;
			MeshData.Normals[VertIndex4] = AverageNormalLeft;
		}
		else
		{
			// If not smoothing we just set the vertex normal to the same normal as the polygon they belong to
			MeshData.Normals[VertIndex1] = MeshData.Normals[VertIndex2] = MeshData.Normals[VertIndex3] = MeshData.Normals[VertIndex4] = NormalCurrent;
		}

		// Tangents (perpendicular to the surface)
		FVector SurfaceTangent = p0 - p1;
		SurfaceTangent = SurfaceTangent.GetSafeNormal();
		MeshData.Tangents[VertIndex1] = MeshData.Tangents[VertIndex2] = MeshData.Tangents[VertIndex3] = MeshData.Tangents[VertIndex4] = FProcMeshTangent(SurfaceTangent, true);
	}
}
