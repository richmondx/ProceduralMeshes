// Example cylinder mesh

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "SimpleCylinderActor.generated.h"

UCLASS()
class PROCEDURALMESHES_API ASimpleCylinderActor : public AActor
{
	GENERATED_BODY()

public:
	ASimpleCylinderActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	float Radius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	float Height = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	int32 RadialSegmentCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	bool bCapEnds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	bool bDoubleSided = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cylinder Parameters")
	bool bSmoothNormals = true;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

private:
	void GenerateMesh();
	void GenerateCylinder(FProceduralMeshData& MeshData, float Height, float InWidth, int32 InCrossSectionCount, bool bCapEnds = false, bool bDoubleSided = false, bool bInSmoothNormals = true);
};
