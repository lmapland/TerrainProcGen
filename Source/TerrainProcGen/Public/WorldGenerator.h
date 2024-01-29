// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGenerator.generated.h"

class UProceduralMeshComponent;
struct FProcMeshTangent;

UCLASS()
class TERRAINPROCGEN_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	AWorldGenerator();
	//virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY);

	UFUNCTION(BlueprintCallable)
	int GenHeight(FVector2D Location);

	UFUNCTION(BlueprintCallable)
	int GenerateHeightLayer(const FVector2D Location, const float InScale, const float InAmplitude, const FVector2D Offset);

	/* For removing the seam between meshes */
	void GetNormalsTangents(TArray<FVector> InVertices, TArray<int> InTriangles, TArray<FVector2D> InUVs, TArray<FVector>& OutNormals, TArray<FProcMeshTangent>& OutTangents, TArray<FVector>& OutVertices, TArray<int>& OutTriangles, TArray<FVector2D>& OutUVs);

	//USceneComponent* SceneComponent1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* TerrainMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* TerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int XVertexCount = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int YVertexCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	float CellSize = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int NumberSectionsX = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int NumberSectionsY = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	int MeshSectionIndex = 0;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleLarge = 0.00001f; //0.00005

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleMedium = 0.0001f; //0.0005

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleSmall = 0.001f; // 0.008

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeLarge = 20000.f; // 10000

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeMedium = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeSmall = 500.f;

};
