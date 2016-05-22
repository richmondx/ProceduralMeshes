// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example heightfield generated with noise

#include "ProceduralMeshesPrivatePCH.h"
#include "HeightFieldNoiseActor.h"

AHeightFieldNoiseActor::AHeightFieldNoiseActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);
}

#if WITH_EDITOR  
void AHeightFieldNoiseActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateMesh();
}
#endif // WITH_EDITOR

void AHeightFieldNoiseActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh();

	// Fix for PCM location/rotation/scale since the whole component is Transient
	ProcMesh->SetWorldLocation(this->GetActorLocation());
	ProcMesh->SetWorldRotation(this->GetActorRotation());
	ProcMesh->SetWorldScale3D(this->GetActorScale3D());
}

void AHeightFieldNoiseActor::GenerateMesh()
{
	if (Length < 1 || Width < 1 || LengthSections < 1 || WidthSections < 1)
	{
		return;
	}

	RngStream = FRandomStream::FRandomStream(RandomSeed);

	// Setup example height data
	int32 NumberOfPoints = (LengthSections + 1) * (WidthSections + 1);
	TArray<float> HeightValues;
	HeightValues.AddUninitialized(NumberOfPoints);

	// Fill height data with random values
	for (int32 i = 0; i < NumberOfPoints; i++)
	{
		HeightValues[i] = RngStream.FRandRange(0, Height);
	}

	// This example re-uses vertices between polygons.
	// TODO: To get non-smooth shading, we need to duplicate vertices for each location so we can set separate normals and tangents for them.
	FProceduralMeshData MeshData = FProceduralMeshData();
	MeshData.Vertices.AddUninitialized(NumberOfPoints);
	MeshData.Triangles.AddUninitialized(LengthSections * WidthSections * 2 * 3); // 2x3 vertex indexes per quad
	MeshData.Normals.AddUninitialized(NumberOfPoints);
	MeshData.UVs.AddUninitialized(NumberOfPoints);
	MeshData.Tangents.AddUninitialized(NumberOfPoints);

	GenerateGrid(MeshData, Length, Width, LengthSections, WidthSections, HeightValues);
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
	ProcMesh->SetMaterial(0, Material);
}

void AHeightFieldNoiseActor::GenerateGrid(FProceduralMeshData& MeshData, float InLength, float InWidth, int32 InLengthSections, int32 InWidthSections, const TArray<float>& InHeightValues)
{
	// Note the coordinates are a bit weird here since I aligned it to the transform (X is forwards or "up", which Y is to the right)
	// Should really fix this up and use standard X, Y coords then transform into object space?
	FVector2D SectionSize = FVector2D(InLength / InLengthSections, InWidth / InWidthSections);
	int32 VertexIndex = 0;
	int32 TriangleIndex = 0;

	for (int32 X = 0; X < InLengthSections + 1; X++)
	{
		for (int32 Y = 0; Y < InWidthSections + 1; Y++)
		{
			// Create a new vertex
			int32 NewVertIndex = VertexIndex++;
			FVector newVertex = FVector(X * SectionSize.X, Y * SectionSize.Y, InHeightValues[NewVertIndex]);
			MeshData.Vertices[NewVertIndex] = newVertex;

			// Note that Unreal UV origin (0,0) is top left
			float U = (float)X / (float)InLengthSections;
			float V = (float)Y / (float)InWidthSections;
			MeshData.UVs[NewVertIndex] = FVector2D(U, V);

			// Once we've created enough verts we can start adding polygons
			if (X > 0 && Y > 0)
			{
				// Each row is InWidthSections+1 number of points.
				// And we have InLength+1 rows
				// Index of current vertex in position is thus: (X * (InWidthSections + 1)) + Y;
				int32 bTopRightIndex = (X * (InWidthSections + 1)) + Y; // Should be same as VertIndex1!
				int32 bTopLeftIndex = bTopRightIndex - 1;
				int32 pBottomRightIndex = ((X - 1) * (InWidthSections + 1)) + Y;
				int32 pBottomLeftIndex = pBottomRightIndex - 1;

				// Now create two triangles from those four vertices
				// The order of these (clockwise/counter-clockwise) dictates which way the normal will face. 
				MeshData.Triangles[TriangleIndex++] = pBottomLeftIndex;
				MeshData.Triangles[TriangleIndex++] = bTopRightIndex;
				MeshData.Triangles[TriangleIndex++] = bTopLeftIndex;

				MeshData.Triangles[TriangleIndex++] = pBottomLeftIndex;
				MeshData.Triangles[TriangleIndex++] = pBottomRightIndex;
				MeshData.Triangles[TriangleIndex++] = bTopRightIndex;
			}
		}
	}
}