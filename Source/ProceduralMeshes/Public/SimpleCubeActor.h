// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example cube mesh

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "SimpleCubeActor.generated.h"

UCLASS()
class PROCEDURALMESHES_API ASimpleCubeActor : public AActor
{
	GENERATED_BODY()

public:
	ASimpleCubeActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Depth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Width = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Height = 100.0f;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

private:
	void GenerateMesh();
	void GenerateCube(FProceduralMeshData& MeshData, float Depth, float Width, float Height);
	int32 BuildQuad(FProceduralMeshData& MeshData, FVector BottomLeft, FVector BottomRight, FVector TopRight, FVector TopLeft, int32 VertexOffset, FVector Normal, FProcMeshTangent Tangent);
};
