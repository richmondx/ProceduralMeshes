// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example heightfield generated with noise

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "HeightFieldNoiseActor.generated.h"

UCLASS()
class PROCEDURALMESHES_API AHeightFieldNoiseActor : public AActor
{
	GENERATED_BODY()

public:
	AHeightFieldNoiseActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Length = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Width = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Height = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 LengthSections = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 WidthSections = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 RandomSeed = 1238;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	UMaterialInterface* Material;

	virtual void BeginPlay() override;


#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

protected:
	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

private:
	void GenerateMesh();
	void GenerateGrid(FProceduralMeshData& MeshData, float InLength, float InWidth, int32 InLengthSections, int32 InWidthSections, const TArray<float>& InHeightValues);

	UPROPERTY(Transient)
	FRandomStream RngStream = FRandomStream::FRandomStream();
};
