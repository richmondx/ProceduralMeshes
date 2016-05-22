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

	FProceduralMeshData MeshData = FProceduralMeshData();
	int32 NumberOfVertices = LengthSections * WidthSections * 4; // 4x vertices per quad/section
	MeshData.Vertices.AddUninitialized(NumberOfVertices);
	MeshData.Triangles.AddUninitialized(LengthSections * WidthSections * 2 * 3); // 2x3 vertex indexes per quad
	MeshData.Normals.AddUninitialized(NumberOfVertices);
	MeshData.UVs.AddUninitialized(NumberOfVertices);
	MeshData.Tangents.AddUninitialized(NumberOfVertices);

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

	for (int32 X = 0; X < InLengthSections; X++)
	{
		for (int32 Y = 0; Y < InWidthSections; Y++)
		{
			// Setup a quad
			int32 BottomLeftIndex = VertexIndex++;
			int32 BottomRightIndex = VertexIndex++;
			int32 TopRightIndex = VertexIndex++;
			int32 TopLeftIndex = VertexIndex++;

			int32 NoiseIndex_BottomLeft = (X * InWidthSections) + Y;
			int32 NoiseIndex_BottomRight = NoiseIndex_BottomLeft + 1;
			int32 NoiseIndex_TopLeft = ((X+1) * InWidthSections) + Y;
			int32 NoiseIndex_TopRight = NoiseIndex_TopLeft + 1;

			FVector pBottomLeft = FVector(X * SectionSize.X, Y * SectionSize.Y, InHeightValues[NoiseIndex_BottomLeft]);
			FVector pBottomRight = FVector(X * SectionSize.X, (Y+1) * SectionSize.Y, InHeightValues[NoiseIndex_BottomRight]);
			FVector pTopRight = FVector((X + 1) * SectionSize.X, (Y + 1) * SectionSize.Y, InHeightValues[NoiseIndex_TopRight]);
			FVector pTopLeft = FVector((X+1) * SectionSize.X, Y * SectionSize.Y, InHeightValues[NoiseIndex_TopLeft]);
			
			MeshData.Vertices[BottomLeftIndex] = pBottomLeft;
			MeshData.Vertices[BottomRightIndex] = pBottomRight;
			MeshData.Vertices[TopRightIndex] = pTopRight;
			MeshData.Vertices[TopLeftIndex] = pTopLeft;

			// Note that Unreal UV origin (0,0) is top left
			MeshData.UVs[BottomLeftIndex] = FVector2D((float)X / (float)InLengthSections, (float)Y / (float)InWidthSections);
			MeshData.UVs[BottomRightIndex] = FVector2D((float)X / (float)InLengthSections, (float)(Y+1) / (float)InWidthSections);
			MeshData.UVs[TopRightIndex] = FVector2D((float)(X+1) / (float)InLengthSections, (float)(Y+1) / (float)InWidthSections);
			MeshData.UVs[TopLeftIndex] = FVector2D((float)(X+1) / (float)InLengthSections, (float)Y / (float)InWidthSections);

			// Now create two triangles from those four vertices
			// The order of these (clockwise/counter-clockwise) dictates which way the normal will face. 
			MeshData.Triangles[TriangleIndex++] = BottomLeftIndex;
			MeshData.Triangles[TriangleIndex++] = TopRightIndex;
			MeshData.Triangles[TriangleIndex++] = TopLeftIndex;

			MeshData.Triangles[TriangleIndex++] = BottomLeftIndex;
			MeshData.Triangles[TriangleIndex++] = BottomRightIndex;
			MeshData.Triangles[TriangleIndex++] = TopRightIndex;

			// Normals
			FVector NormalCurrent = FVector::CrossProduct(MeshData.Vertices[BottomLeftIndex] - MeshData.Vertices[TopLeftIndex], MeshData.Vertices[TopRightIndex] - MeshData.Vertices[TopLeftIndex]).GetSafeNormal();

			// If not smoothing we just set the vertex normal to the same normal as the polygon they belong to
			MeshData.Normals[BottomLeftIndex] = MeshData.Normals[BottomRightIndex] = MeshData.Normals[TopRightIndex] = MeshData.Normals[TopLeftIndex] = NormalCurrent;

			// Tangents (perpendicular to the surface)
			FVector SurfaceTangent = pBottomLeft - pBottomRight;
			SurfaceTangent = SurfaceTangent.GetSafeNormal();
			MeshData.Tangents[BottomLeftIndex] = MeshData.Tangents[BottomRightIndex] = MeshData.Tangents[TopRightIndex] = MeshData.Tangents[TopLeftIndex] = FProcMeshTangent(SurfaceTangent, true);
		}
	}
}