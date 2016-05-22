// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example cylinder mesh

#include "ProceduralMeshesPrivatePCH.h"
#include "SimpleCylinderActor.h"

ASimpleCylinderActor::ASimpleCylinderActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);
}

#if WITH_EDITOR  
void ASimpleCylinderActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateMesh();
}
#endif // WITH_EDITOR

void ASimpleCylinderActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh();

	// Fix for PCM location/rotation/scale since the whole component is Transient
	ProcMesh->SetWorldLocation(this->GetActorLocation());
	ProcMesh->SetWorldRotation(this->GetActorRotation());
	ProcMesh->SetWorldScale3D(this->GetActorScale3D());
}

void ASimpleCylinderActor::GenerateMesh()
{
	if (Height <= 0)
	{
		return;
	}

	FProceduralMeshData MeshData = FProceduralMeshData();
	GenerateCylinder(MeshData, Height, Radius, RadialSegmentCount, bCapEnds, bDoubleSided, bSmoothNormals);
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
	ProcMesh->SetMaterial(0, Material);
}

void ASimpleCylinderActor::GenerateCylinder(FProceduralMeshData& MeshData, float InHeight, float InWidth, int32 InCrossSectionCount, bool bInCapEnds, bool bInDoubleSided, bool bInSmoothNormals/* = true*/)
{
	// -------------------------------------------------------
	// Basic setup
	int32 VertexIndex = 0;
	int32 NumVerts = InCrossSectionCount * 4; // 4 verts per face

	 // Count extra vertices if double sided
	if (bInDoubleSided)
	{
		NumVerts = NumVerts * 2;
	}

	// Count vertices for caps if set
	if (bInCapEnds)
	{
		// Each cap adds as many verts as there are verts in a circle
		// 2x Number of vertices x3
		NumVerts += 2 * (InCrossSectionCount - 1) * 3;
	}

	MeshData.Vertices.AddUninitialized(NumVerts);
	MeshData.Normals.AddUninitialized(NumVerts);
	MeshData.Tangents.AddUninitialized(NumVerts);
	MeshData.UVs.AddUninitialized(NumVerts);

	// -------------------------------------------------------
	// Make a cylinder section
	const float AngleBetweenQuads = (2.0f / (float)(InCrossSectionCount)) * PI;
	const float VMapPerQuad = 1.0f / (float)InCrossSectionCount;
	FVector Offset = FVector(0, 0, InHeight);

	// Start by building up vertices that make up the cylinder sides
	for (int32 QuadIndex = 0; QuadIndex < InCrossSectionCount; QuadIndex++)
	{
		float Angle = (float)QuadIndex * AngleBetweenQuads;
		float NextAngle = (float)(QuadIndex + 1) * AngleBetweenQuads;

		// Set up the vertices
		FVector p0 = FVector(FMath::Cos(Angle) * InWidth, FMath::Sin(Angle) * InWidth, 0.f);
		FVector p1 = FVector(FMath::Cos(NextAngle) * InWidth, FMath::Sin(NextAngle) * InWidth, 0.f);
		FVector p2 = p1 + Offset;
		FVector p3 = p0 + Offset;

		// Set up the quad triangles
		int32 VertIndex1 = VertexIndex++;
		int32 VertIndex2 = VertexIndex++;
		int32 VertIndex3 = VertexIndex++;
		int32 VertIndex4 = VertexIndex++;

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
		MeshData.UVs[VertIndex1] = FVector2D(1.0f - (VMapPerQuad * QuadIndex), 1.0f);
		MeshData.UVs[VertIndex2] = FVector2D(1.0f - (VMapPerQuad * (QuadIndex + 1)), 1.0f);
		MeshData.UVs[VertIndex3] = FVector2D(1.0f - (VMapPerQuad * (QuadIndex + 1)), 0.0f);
		MeshData.UVs[VertIndex4] = FVector2D(1.0f - (VMapPerQuad * QuadIndex), 0.0f);

		// Normals
		FVector NormalCurrent = FVector::CrossProduct(MeshData.Vertices[VertIndex1] - MeshData.Vertices[VertIndex3], MeshData.Vertices[VertIndex2] - MeshData.Vertices[VertIndex3]).GetSafeNormal();

		if (bInSmoothNormals)
		{
			// To smooth normals we give the vertices different values than the polygon they belong to.
			// GPUs know how to interpolate between those.
			// I do this here as an average between normals of two adjacent polygons
			float NextNextAngle = (float)(QuadIndex + 2) * AngleBetweenQuads;
			FVector p4 = FVector(FMath::Cos(NextNextAngle) * InWidth, FMath::Sin(NextNextAngle) * InWidth, 0.f);

			// p1 to p4 to p2
			FVector NormalNext = FVector::CrossProduct(p1 - p2, p4 - p2).GetSafeNormal();
			FVector AverageNormalRight = (NormalCurrent + NormalNext) / 2;
			AverageNormalRight = AverageNormalRight.GetSafeNormal();

			float PreviousAngle = (float)(QuadIndex - 1) * AngleBetweenQuads;
			FVector pMinus1 = FVector(FMath::Cos(PreviousAngle) * InWidth, FMath::Sin(PreviousAngle) * InWidth, 0.f);

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

		// -------------------------------------------------------
		// If double sided, create extra polygons but face the normals the other way.
		if (bInDoubleSided)
		{
			VertIndex1 = VertexIndex++;
			VertIndex2 = VertexIndex++;
			VertIndex3 = VertexIndex++;
			VertIndex4 = VertexIndex++;

			MeshData.Vertices[VertIndex1] = p0;
			MeshData.Vertices[VertIndex2] = p1;
			MeshData.Vertices[VertIndex3] = p2;
			MeshData.Vertices[VertIndex4] = p3;

			// Reverse the poly order to face them the other way
			MeshData.Triangles.Add(VertIndex4);
			MeshData.Triangles.Add(VertIndex1);
			MeshData.Triangles.Add(VertIndex3);

			MeshData.Triangles.Add(VertIndex3);
			MeshData.Triangles.Add(VertIndex1);
			MeshData.Triangles.Add(VertIndex2);

			// UVs  (Unreal 1,1 is top left)
			MeshData.UVs[VertIndex1] = FVector2D(1.0f - (VMapPerQuad * QuadIndex), 1.0f);
			MeshData.UVs[VertIndex2] = FVector2D(1.0f - (VMapPerQuad * (QuadIndex + 1)), 1.0f);
			MeshData.UVs[VertIndex3] = FVector2D(1.0f - (VMapPerQuad * (QuadIndex + 1)), 0.0f);
			MeshData.UVs[VertIndex4] = FVector2D(1.0f - (VMapPerQuad * QuadIndex), 0.0f);

			// Just simple (unsmoothed) normal for these
			MeshData.Normals[VertIndex1] = MeshData.Normals[VertIndex2] = MeshData.Normals[VertIndex3] = MeshData.Normals[VertIndex4] = NormalCurrent;

			// Tangents (perpendicular to the surface)
			FVector SurfaceTangent = p0 - p1;
			SurfaceTangent = SurfaceTangent.GetSafeNormal();
			MeshData.Tangents[VertIndex1] = MeshData.Tangents[VertIndex2] = MeshData.Tangents[VertIndex3] = MeshData.Tangents[VertIndex4] = FProcMeshTangent(SurfaceTangent, true);
		}

		// -------------------------------------------------------
		// Caps are closed here by triangles that start at 0, then use the points along the circle for the other two corners.
		// A better looking method uses a vertex in the center of the circle, but uses two more polygons.  We will demonstrate that in a different sample.
		if (QuadIndex != 0 && bInCapEnds)
		{
			// Bottom cap
			FVector capVertex0 = FVector(FMath::Cos(0) * InWidth, FMath::Sin(0) * InWidth, 0.f);
			FVector capVertex1 = FVector(FMath::Cos(Angle) * InWidth, FMath::Sin(Angle) * InWidth, 0.f);
			FVector capVertex2 = FVector(FMath::Cos(NextAngle) * InWidth, FMath::Sin(NextAngle) * InWidth, 0.f);

			VertIndex1 = VertexIndex++;
			VertIndex2 = VertexIndex++;
			VertIndex3 = VertexIndex++;
			MeshData.Vertices[VertIndex1] = capVertex0;
			MeshData.Vertices[VertIndex2] = capVertex1;
			MeshData.Vertices[VertIndex3] = capVertex2;

			MeshData.Triangles.Add(VertIndex1);
			MeshData.Triangles.Add(VertIndex2);
			MeshData.Triangles.Add(VertIndex3);

			FVector2D UV1 = FVector2D(0.5f - (FMath::Cos(0) / 2.0f), 0.5f - (FMath::Sin(0) / 2.0f));
			FVector2D UV2 = FVector2D(0.5f - (FMath::Cos(-Angle) / 2.0f), 0.5f - (FMath::Sin(-Angle) / 2.0f));
			FVector2D UV3 = FVector2D(0.5f - (FMath::Cos(-NextAngle) / 2.0f), 0.5f - (FMath::Sin(-NextAngle) / 2.0f));

			MeshData.UVs[VertIndex1] = UV1;
			MeshData.UVs[VertIndex2] = UV2;
			MeshData.UVs[VertIndex3] = UV3;

			FVector NormalCurrent = FVector::CrossProduct(MeshData.Vertices[VertIndex1] - MeshData.Vertices[VertIndex3], MeshData.Vertices[VertIndex2] - MeshData.Vertices[VertIndex3]).GetSafeNormal();
			MeshData.Normals[VertIndex1] = MeshData.Normals[VertIndex2] = MeshData.Normals[VertIndex3] = NormalCurrent;

			// Tangents (perpendicular to the surface)
			FVector SurfaceTangent = p0 - p1;
			SurfaceTangent = SurfaceTangent.GetSafeNormal();
			MeshData.Tangents[VertIndex1] = MeshData.Tangents[VertIndex2] = MeshData.Tangents[VertIndex3] = FProcMeshTangent(SurfaceTangent, true);

			// Top cap
			capVertex0 = capVertex0 + Offset;
			capVertex1 = capVertex1 + Offset;
			capVertex2 = capVertex2 + Offset;

			VertIndex1 = VertexIndex++;
			VertIndex2 = VertexIndex++;
			VertIndex3 = VertexIndex++;
			MeshData.Vertices[VertIndex1] = capVertex0;
			MeshData.Vertices[VertIndex2] = capVertex1;
			MeshData.Vertices[VertIndex3] = capVertex2;

			MeshData.Triangles.Add(VertIndex3);
			MeshData.Triangles.Add(VertIndex2);
			MeshData.Triangles.Add(VertIndex1);

			UV1 = FVector2D(0.5f - (FMath::Cos(0) / 2.0f), 0.5f - (FMath::Sin(0) / 2.0f));
			UV2 = FVector2D(0.5f - (FMath::Cos(Angle) / 2.0f), 0.5f - (FMath::Sin(Angle) / 2.0f));
			UV3 = FVector2D(0.5f - (FMath::Cos(NextAngle) / 2.0f), 0.5f - (FMath::Sin(NextAngle) / 2.0f));

			MeshData.UVs[VertIndex1] = UV1;
			MeshData.UVs[VertIndex2] = UV2;
			MeshData.UVs[VertIndex3] = UV3;

			NormalCurrent = FVector::CrossProduct(MeshData.Vertices[VertIndex1] - MeshData.Vertices[VertIndex3], MeshData.Vertices[VertIndex2] - MeshData.Vertices[VertIndex3]).GetSafeNormal();
			MeshData.Normals[VertIndex1] = MeshData.Normals[VertIndex2] = MeshData.Normals[VertIndex3] = NormalCurrent;

			// Tangents (perpendicular to the surface)
			SurfaceTangent = p0 - p1;
			SurfaceTangent = SurfaceTangent.GetSafeNormal();
			MeshData.Tangents[VertIndex1] = MeshData.Tangents[VertIndex2] = MeshData.Tangents[VertIndex3] = FProcMeshTangent(SurfaceTangent, true);
		}
	}
}
