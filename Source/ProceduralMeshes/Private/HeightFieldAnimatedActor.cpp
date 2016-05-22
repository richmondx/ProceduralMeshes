// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example heightfield grid animated with sine and cosine waves

#include "ProceduralMeshesPrivatePCH.h"
#include "HeightFieldAnimatedActor.h"

AHeightFieldAnimatedActor::AHeightFieldAnimatedActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProcMesh->AttachTo(RootComponent);
	// Make sure the PMC doesnt save any mesh data with the map
	ProcMesh->SetFlags(EObjectFlags::RF_Transient);

	PrimaryActorTick.bCanEverTick = true;
}

#if WITH_EDITOR  
void AHeightFieldAnimatedActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateMesh();
}
#endif // WITH_EDITOR

void AHeightFieldAnimatedActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh();

	// Fix for PCM location/rotation/scale since the whole component is Transient
	ProcMesh->SetWorldLocation(this->GetActorLocation());
	ProcMesh->SetWorldRotation(this->GetActorRotation());
	ProcMesh->SetWorldScale3D(this->GetActorScale3D());
}

void AHeightFieldAnimatedActor::Tick(float DeltaSeconds)
{
	if (AnimateMesh)
	{
		CurrentAnimationFrameX += DeltaSeconds * AnimationSpeedX;
		CurrentAnimationFrameY += DeltaSeconds * AnimationSpeedY;
		GenerateMesh();
	}
}

void AHeightFieldAnimatedActor::GenerateMesh()
{
	if (Length < 1 || Width < 1 || LengthSections < 1 || WidthSections < 1)
	{
		return;
	}

	// Setup example height data
	int32 NumberOfPoints = (LengthSections + 1) * (WidthSections + 1);
	TArray<float> HeightValues;
	HeightValues.AddUninitialized(NumberOfPoints);

	// Combine variations of sine and cosine to create some variable waves
	int32 VertexIndex = 0;

	for (int32 X = 0; X < LengthSections + 1; X++)
	{
		for (int32 Y = 0; Y < WidthSections + 1; Y++)
		{
			// Just some quick hardcoded offset numbers in there
			float ValueOne = FMath::Cos((X + CurrentAnimationFrameX)*ScaleFactor) + FMath::Sin((Y + CurrentAnimationFrameY)*ScaleFactor);
			float ValueTwo = FMath::Cos((X + CurrentAnimationFrameX*0.7f)*ScaleFactor*2.5f) + FMath::Sin((Y - CurrentAnimationFrameY*0.7f)*ScaleFactor*2.5f);
			HeightValues[VertexIndex++] = ((ValueOne + ValueTwo) / 2) * Height;
		}
	}

	// This example re-uses vertices between polygons.
	FProceduralMeshData MeshData = FProceduralMeshData();
	MeshData.Vertices.AddUninitialized(NumberOfPoints);
	MeshData.Triangles.AddUninitialized(LengthSections * WidthSections * 2 * 3); // 2x3 vertex per quad
	MeshData.Normals.AddUninitialized(NumberOfPoints);
	MeshData.UVs.AddUninitialized(NumberOfPoints);
	MeshData.Tangents.AddUninitialized(NumberOfPoints);

	GenerateGrid(MeshData, Length, Width, LengthSections, WidthSections, HeightValues);
	ProcMesh->ClearAllMeshSections();
	ProcMesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UVs, MeshData.VertexColors, MeshData.Tangents, false);
	ProcMesh->SetMaterial(0, Material);
}

void AHeightFieldAnimatedActor::GenerateGrid(FProceduralMeshData& MeshData, float InLength, float InWidth, int32 InLengthSections, int32 InWidthSections, const TArray<float>& InHeightValues)
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