// Example Sierpinski pyramid using cylinder lines

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "SierpinskiLineActor.generated.h"

// A simple struct to keep some data together
USTRUCT()
struct FPyramidLine
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Start;

	UPROPERTY()
	FVector End;

	UPROPERTY()
	float Width;

	FPyramidLine()
	{
		Start = FVector::ZeroVector;
		End = FVector::ZeroVector;
		Width = 1.0f;
	}

	FPyramidLine(FVector InStart, FVector InEnd)
	{
		Start = InStart;
		End = InEnd;
		Width = 1.0f;
	}

	FPyramidLine(FVector InStart, FVector InEnd, float InWidth)
	{
		Start = InStart;
		End = InEnd;
		Width = InWidth;
	}
};

UCLASS()
class PROCEDURALMESHES_API ASierpinskiLineActor : public AActor
{
	GENERATED_BODY()

public:
	ASierpinskiLineActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Size = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 Iterations = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float ThicknessMultiplierPerGeneration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 RadialSegmentCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	bool bSmoothNormals = false;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

private:
	void GenerateMesh();

	UPROPERTY(Transient)
	TArray<FPyramidLine> Lines;

	FVector RotatePointAroundPivot(FVector InPoint, FVector InPivot, FVector InAngles);
	void PreCacheCrossSection();
	void AddSection(FVector InBottomLeftPoint, FVector InTopPoint, FVector InBottomRightPoint, FVector InBottomMiddlePoint, int InDepth);
	void GenerateCylinder(FProceduralMeshData& MeshData, FVector StartPoint, FVector EndPoint, float InWidth, int32 InCrossSectionCount, int InVertexIndexStart, bool bInSmoothNormals = true);

	int LastCachedCrossSectionCount;
	UPROPERTY(Transient)
	TArray<FVector> CachedCrossSectionPoints;
};
