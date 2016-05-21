// Example cylinder strip mesh

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "CylinderStripActor.generated.h"

UCLASS()
class PROCEDURALMESHES_API ACylinderStripActor : public AActor
{
	GENERATED_BODY()

public:
	ACylinderStripActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshParameters)
	TArray<FVector> LinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	float Radius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	int32 RadialSegmentCount = 10;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

private:
	void GenerateMesh();

	FVector RotatePointAroundPivot(FVector InPoint, FVector InPivot, FVector InAngles);
	void PreCacheCrossSection();
	void GenerateCylinder(FProceduralMeshData& MeshData, FVector StartPoint, FVector EndPoint, float InWidth, int32 InCrossSectionCount, int InVertexIndexStart, bool bInSmoothNormals = true);

	int LastCachedCrossSectionCount;
	UPROPERTY(Transient)
	TArray<FVector> CachedCrossSectionPoints;
};
