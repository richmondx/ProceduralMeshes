// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example cube

#include "ProceduralMeshesPrivatePCH.h"
#include "SimpleCubeActor.h"

ASimpleCubeActor::ASimpleCubeActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);
}

#if WITH_EDITOR  
void ASimpleCubeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateMesh();
}
#endif // WITH_EDITOR

void ASimpleCubeActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh();
}

void ASimpleCubeActor::GenerateMesh()
{
	FProceduralMeshData MeshData = FProceduralMeshData();
	GenerateCube(MeshData, Depth, Width, Height);
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
}

void ASimpleCubeActor::GenerateCube(FProceduralMeshData& MeshData, float Depth, float Width, float Height)
{
	// NOTE: Unreal uses an upper-left origin UV
	// NOTE: This sample uses a simple UV mapping scheme where each face is the same
	// NOTE: For a normal facing towards us, be build the polygon CCW in the order 0-1-2 then 0-2-3 to complete the quad.
	// Remember in Unreal, X is forwards, Y is to your right and Z is up.

	// Calculate a half offset so we get correct center of object
	float DepthOffset = Depth / 2.0f; // X
	float WidthOffset = Width / 2.0f; // Y
	float HeightOffset = Height / 2.0f; // Z

	// Define the 8 corners of the cube
	FVector p0 = FVector(DepthOffset,  WidthOffset, -HeightOffset);
	FVector p1 = FVector(DepthOffset, -WidthOffset, -HeightOffset);
	FVector p2 = FVector(DepthOffset, -WidthOffset,  HeightOffset);
	FVector p3 = FVector(DepthOffset,  WidthOffset,  HeightOffset);
	FVector p4 = FVector(-DepthOffset, WidthOffset, -HeightOffset);
	FVector p5 = FVector(-DepthOffset, -WidthOffset, -HeightOffset);
	FVector p6 = FVector(-DepthOffset, -WidthOffset, HeightOffset);
	FVector p7 = FVector(-DepthOffset, WidthOffset, HeightOffset);

	// Now we create 6x faces, 4 vertices each
	int32 VertexCount = 6 * 4;
	MeshData.Vertices.AddUninitialized(VertexCount);
	MeshData.UVs.AddUninitialized(VertexCount);
	MeshData.Normals.AddUninitialized(VertexCount);
	MeshData.Tangents.AddUninitialized(VertexCount);

	int32 VertexOffset = 0;
	FVector Normal = FVector::ZeroVector;
	FProcMeshTangent Tangent = FProcMeshTangent();

 	// Front (+X) face: 0-1-2-3
	Normal = FVector(1, 0, 0);
	Tangent = FProcMeshTangent(0, 1, 0);
	VertexOffset = BuildQuad(MeshData, p0, p1, p2, p3, VertexOffset, Normal, Tangent);

 	// Back (-X) face: 5-4-7-6
	Normal = FVector(-1, 0, 0);
	Tangent = FProcMeshTangent(0, -1, 0);
	VertexOffset = BuildQuad(MeshData, p5, p4, p7, p6, VertexOffset, Normal, Tangent);

 	// Left (-Y) face: 1-5-6-2
	Normal = FVector(0, -1, 0);
	Tangent = FProcMeshTangent(1, 0, 0);
	VertexOffset = BuildQuad(MeshData, p1, p5, p6, p2, VertexOffset, Normal, Tangent);

 	// Right (+Y) face: 4-0-3-7
	Normal = FVector(0, 1, 0);
	Tangent = FProcMeshTangent(-1, 0, 0);
	VertexOffset = BuildQuad(MeshData, p4, p0, p3, p7, VertexOffset, Normal, Tangent);

 	// Top (+Z) face: 6-7-3-2
	Normal = FVector(0, 0, 1);
	Tangent = FProcMeshTangent(0, 1, 0);
	VertexOffset = BuildQuad(MeshData, p6, p7, p3, p2, VertexOffset, Normal, Tangent);

 	// Bottom (-Z) face: 1-0-4-5
	Normal = FVector(0, 0, -1);
	Tangent = FProcMeshTangent(0, -1, 0);
	VertexOffset = BuildQuad(MeshData, p1, p0, p4, p5, VertexOffset, Normal, Tangent);
}

int32 ASimpleCubeActor::BuildQuad(FProceduralMeshData& MeshData, FVector BottomLeft, FVector BottomRight, FVector TopRight, FVector TopLeft, int32 VertexOffset, FVector Normal, FProcMeshTangent Tangent)
{
	MeshData.Vertices[VertexOffset + 0] = BottomLeft;
	MeshData.Vertices[VertexOffset + 1] = BottomRight;
	MeshData.Vertices[VertexOffset + 2] = TopRight;
	MeshData.Vertices[VertexOffset + 3] = TopLeft;
	MeshData.UVs[VertexOffset + 0] = FVector2D(0.0f, 1.0f);
	MeshData.UVs[VertexOffset + 1] = FVector2D(1.0f, 1.0f);
	MeshData.UVs[VertexOffset + 2] = FVector2D(1.0f, 0.0f);
	MeshData.UVs[VertexOffset + 3] = FVector2D(0.0f, 0.0f);
	MeshData.Triangles.Add(VertexOffset + 0);
	MeshData.Triangles.Add(VertexOffset + 1);
	MeshData.Triangles.Add(VertexOffset + 2);
	MeshData.Triangles.Add(VertexOffset + 0);
	MeshData.Triangles.Add(VertexOffset + 2);
	MeshData.Triangles.Add(VertexOffset + 3);
	// On a cube, all the vertex normals face the same way
	MeshData.Normals[VertexOffset + 0] = MeshData.Normals[VertexOffset + 1] = MeshData.Normals[VertexOffset + 2] = MeshData.Normals[VertexOffset + 3] = Normal;
	MeshData.Tangents[VertexOffset + 0] = MeshData.Tangents[VertexOffset + 1] = MeshData.Tangents[VertexOffset + 2] = MeshData.Tangents[VertexOffset + 3] = Tangent;
	return VertexOffset + 4;
}