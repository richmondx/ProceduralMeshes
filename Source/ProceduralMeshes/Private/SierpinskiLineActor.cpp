// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example Sierpinski pyramid using cylinder lines

#include "ProceduralMeshesPrivatePCH.h"
#include "SierpinskiLineActor.h"

ASierpinskiLineActor::ASierpinskiLineActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);
}

#if WITH_EDITOR  
void ASierpinskiLineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	PreCacheCrossSection();
	GenerateMesh();
}
#endif // WITH_EDITOR

void ASierpinskiLineActor::BeginPlay()
{
	Super::BeginPlay();
	PreCacheCrossSection();
	GenerateMesh();
}

void ASierpinskiLineActor::GenerateMesh()
{
	Lines.Empty();

	// -------------------------------------------------------
	// Start by setting the four points that define a pyramid
	// 0,0 is center bottom.. so first two are offset half Size to the sides, and the 3rd straight up
	FVector BottomLeftPoint = FVector(0, -0.5f * Size, 0);
	FVector BottomRightPoint = FVector(0, 0.5f * Size, 0);
	float ThirdBasePointDistance = FMath::Sqrt(3) * Size / 2;
	FVector BottomMiddlePoint = FVector(ThirdBasePointDistance, 0, 0);
	float CenterPosX = FMath::Tan(FMath::DegreesToRadians(30)) * (Size / 2.0f);
	FVector TopPoint = FVector(CenterPosX, 0, ThirdBasePointDistance);

	// Then create all the lines between those 4 points
	Lines.Add(FPyramidLine(BottomLeftPoint, BottomRightPoint, LineThickness));
	Lines.Add(FPyramidLine(BottomRightPoint, TopPoint, LineThickness));
	Lines.Add(FPyramidLine(TopPoint, BottomLeftPoint, LineThickness));

	Lines.Add(FPyramidLine(BottomLeftPoint, BottomMiddlePoint, LineThickness));
	Lines.Add(FPyramidLine(BottomMiddlePoint, BottomRightPoint, LineThickness));
	Lines.Add(FPyramidLine(BottomMiddlePoint, TopPoint, LineThickness));

	// -------------------------------------------------------
	// Create the rest of the lines through recursion
	AddSection(BottomLeftPoint, TopPoint, BottomRightPoint, BottomMiddlePoint, 1);

	// -------------------------------------------------------
	// Calculate and pre-allocate buffers
	int TotalNumberOfVerticesPerSection = RadialSegmentCount * 4; // 4 verts per face 
	int TotalNumberOfTrianglesPerSection = TotalNumberOfVerticesPerSection + 2 * RadialSegmentCount;

	FProceduralMeshData MeshData = FProceduralMeshData();
	MeshData.Vertices.AddUninitialized(TotalNumberOfVerticesPerSection * Lines.Num());
	MeshData.Triangles.AddUninitialized(TotalNumberOfTrianglesPerSection * Lines.Num());
	MeshData.Normals.AddUninitialized(TotalNumberOfVerticesPerSection * Lines.Num());
	MeshData.UVs.AddUninitialized(TotalNumberOfVerticesPerSection * Lines.Num());
	MeshData.Tangents.AddUninitialized(TotalNumberOfVerticesPerSection * Lines.Num());

	// -------------------------------------------------------
	// Now lets loop through all the defined lines of the pyramid and create a cylinder for each
	int CurrentIndexStart = 0;

	for (int i = 0; i < Lines.Num(); i++)
	{
		CurrentIndexStart = i * TotalNumberOfVerticesPerSection;
		GenerateCylinder(MeshData, Lines[i].Start, Lines[i].End, Lines[i].Width, RadialSegmentCount, CurrentIndexStart, bSmoothNormals);
	}
	
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
	ProcMesh->SetMaterial(0, Material);
}

FVector ASierpinskiLineActor::RotatePointAroundPivot(FVector InPoint, FVector InPivot, FVector InAngles)
{
	FVector direction = InPoint - InPivot; // get point direction relative to pivot
	direction = FQuat::MakeFromEuler(InAngles) * direction; // rotate it
	return direction + InPivot; // calculate rotated point
}

void ASierpinskiLineActor::PreCacheCrossSection()
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

void ASierpinskiLineActor::AddSection(FVector InBottomLeftPoint, FVector InTopPoint, FVector InBottomRightPoint, FVector InBottomMiddlePoint, int InDepth)
{
	if (InDepth > Iterations)
	{
		return;
	}

	// First side
	FVector Side1LeftPoint = InTopPoint - InBottomLeftPoint;
	Side1LeftPoint = (Side1LeftPoint * 0.5f) + InBottomLeftPoint;
	FVector Side1RightPoint = InBottomRightPoint - InTopPoint;
	Side1RightPoint = (Side1RightPoint * 0.5f) + InTopPoint;
	FVector Side1BottomPoint = InBottomLeftPoint - InBottomRightPoint;
	Side1BottomPoint = (Side1BottomPoint * 0.5f) + InBottomRightPoint;

	// Points Towards Middle
	FVector MiddlePointUp = InBottomMiddlePoint - InTopPoint;
	MiddlePointUp = (MiddlePointUp * 0.5f) + InTopPoint;
	FVector BottomLeftPoint = InBottomMiddlePoint - InBottomLeftPoint;
	BottomLeftPoint = (BottomLeftPoint * 0.5f) + InBottomLeftPoint;
	FVector BottomRightPoint = InBottomMiddlePoint - InBottomRightPoint;
	BottomRightPoint = (BottomRightPoint * 0.5f) + InBottomRightPoint;

	// Find new thickness
	float NewThickness = LineThickness * FMath::Pow(ThicknessMultiplierPerGeneration, InDepth);

	// First side
	Lines.Add(FPyramidLine(Side1LeftPoint, Side1RightPoint, NewThickness));
	Lines.Add(FPyramidLine(Side1RightPoint, Side1BottomPoint, NewThickness));
	Lines.Add(FPyramidLine(Side1BottomPoint, Side1LeftPoint, NewThickness));

	// Second side
	Lines.Add(FPyramidLine(BottomLeftPoint, Side1LeftPoint, NewThickness));
	Lines.Add(FPyramidLine(BottomLeftPoint, MiddlePointUp, NewThickness));
	Lines.Add(FPyramidLine(Side1LeftPoint, MiddlePointUp, NewThickness));

	// Third side
	Lines.Add(FPyramidLine(BottomRightPoint, Side1RightPoint, NewThickness));
	Lines.Add(FPyramidLine(BottomRightPoint, MiddlePointUp, NewThickness));
	Lines.Add(FPyramidLine(Side1RightPoint, MiddlePointUp, NewThickness));

	// Fourth side (bottom)
	Lines.Add(FPyramidLine(Side1BottomPoint, BottomLeftPoint, NewThickness));
	Lines.Add(FPyramidLine(Side1BottomPoint, BottomRightPoint, NewThickness));
	Lines.Add(FPyramidLine(BottomLeftPoint, BottomRightPoint, NewThickness));

	AddSection(InBottomLeftPoint, Side1LeftPoint, Side1BottomPoint, BottomLeftPoint, InDepth + 1); // Lower left pyramid
	AddSection(Side1LeftPoint, InTopPoint, Side1RightPoint, MiddlePointUp, InDepth + 1); // Top pyramid
	AddSection(Side1BottomPoint, Side1RightPoint, InBottomRightPoint, BottomRightPoint, InDepth + 1); // Lower right pyramid
	AddSection(BottomLeftPoint, MiddlePointUp, BottomRightPoint, InBottomMiddlePoint, InDepth + 1); // Lower middle pyramid
}

void ASierpinskiLineActor::GenerateCylinder(FProceduralMeshData& MeshData, FVector StartPoint, FVector EndPoint, float InWidth, int32 InCrossSectionCount, int InVertexIndexStart, bool bInSmoothNormals/* = true*/)
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
