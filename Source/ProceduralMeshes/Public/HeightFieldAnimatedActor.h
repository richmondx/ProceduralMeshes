// Copyright 2016, Sigurdur Gunnarsson. All Rights Reserved. 
// Example heightfield grid animated with sine and cosine waves

#pragma once

#include "ProceduralMeshesPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshData.h"
#include "HeightFieldAnimatedActor.generated.h"

UCLASS()
class PROCEDURALMESHES_API AHeightFieldAnimatedActor : public AActor
{
	GENERATED_BODY()

public:
	AHeightFieldAnimatedActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Length = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Width = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float Height = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float ScaleFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 LengthSections = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 WidthSections = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	int32 RandomSeed = 1238;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	bool AnimateMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float AnimationSpeedX = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Parameters")
	float AnimationSpeedY = 4.5f;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif   // WITH_EDITOR

protected:
	UPROPERTY(Transient, DuplicateTransient)
	UProceduralMeshComponent* ProcMesh;

	float CurrentAnimationFrameX = 0.0f;
	float CurrentAnimationFrameY = 0.0f;

private:
	void GenerateMesh();
	void GenerateGrid(FProceduralMeshData& MeshData, float InLength, float InWidth, int32 InLengthSections, int32 InWidthSections, const TArray<float>& InHeightValues);
};
