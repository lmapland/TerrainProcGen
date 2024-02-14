// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "WorldEngine.generated.h"

class UProceduralMeshComponent;
class UFoliageType_InstancedStaticMesh;

USTRUCT(BlueprintType)
struct FFoliageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<int> Instances;
};

UCLASS()
class TERRAINPG_API AWorldEngine : public AActor
{
	GENERATED_BODY()
	
public:
	AWorldEngine();

	UFUNCTION(BlueprintCallable)
	void GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY, const int LODFactor);

	UFUNCTION(BlueprintCallable)
	int DrawTile();

	UFUNCTION(BlueprintImplementableEvent)
	bool RemoveFoliageTile(const int TileIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveFoliageTile2(const int TileIndex);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void GenerateTerrainAsync(const int InSectionIndexX, const int InSectionIndexY, const int InLODLevel);

	UFUNCTION(BlueprintCallable)
	int GenHeight(FVector2D Location);

	UFUNCTION(BlueprintCallable)
	int GenerateHeightLayer(const FVector2D Location, const float InScale, const float InAmplitude, const FVector2D Offset);

	/* For removing the seam between meshes */
	void GetNormalsTangents(TArray<FVector> InVertices, TArray<FVector2D> InUVs, FVector2D InLODVertexCount, float InLODCellSize);
	
	UFUNCTION(BlueprintCallable)
	FVector GetPlayerLocation();

	UFUNCTION(BlueprintCallable)
	FVector2D GetTileLocation(FIntPoint TileCoordinate);

	UFUNCTION(BlueprintCallable)
	FIntPoint GetClosestQueuedTile();

	UFUNCTION(BlueprintCallable)
	int GetFurthestUpdatableTileIndex();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitializeFoliageTypes();

	UFUNCTION(BlueprintCallable)
	void AddFoliageInstances(const FVector InLocation);

	UFUNCTION(BlueprintCallable)
	void SpawnFoliageCluster(UFoliageType_InstancedStaticMesh* FoliageType, UInstancedStaticMeshComponent* FoliageIsmComponent, const FVector ClusterLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	UProceduralMeshComponent* TerrainMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	UMaterialInterface* TerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int XVertexCount = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int YVertexCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	float CellSize = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int NumberSectionsX = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Procedural Generation|Defaults")
	int NumberSectionsY = 4;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	int MeshSectionIndex = 0;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleLarge = 20000.f; //0.00001f

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleMedium = 3000.f; //0.0001f

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float ScaleSmall = 200.f; //0.001f

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeLarge = 4000.f; // 10000

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeMedium = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Height Params")
	float AmplitudeSmall = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	bool bRandomizeLayout = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	FVector2D PerlinOffset = FVector2D(0.6);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	TMap<FIntPoint, FIntPoint> QueuedTiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Defaults")
	float TileDiscardDistance;

	int SectionIndexX = 0;
	int SectionIndexY = 0;
	int LODLevel = 1;

	TArray<int> Triangles;
	TArray<int> SubTriangles;
	TArray<FVector> SubNormals;
	TArray<FProcMeshTangent> SubTangents;
	TArray<FVector2D> SubUVs;
	TArray<FVector> SubVertices;

	UPROPERTY(BlueprintReadWrite)
	bool bGeneratorBusy = false;

	UPROPERTY(BlueprintReadOnly)
	bool bTileDataReady = false;

	/* Foliage variables */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	TArray<UFoliageType_InstancedStaticMesh*> FoliageTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	bool RandomizeFoliage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int InitialFoliageSeed = 27;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	FRandomStream FoliageSeed;

	UPROPERTY(BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	TArray<FVector> FoliagePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int GrowthProbabilityPercentage = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int MaxClusterSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int InstanceOffset = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int InstanceOffsetVariation = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	FVector InstanceScale = FVector(1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	float ScaleVariationMultiplier = .2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	int FoliageSpawnPercentage = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	TArray<UInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation|Foliage")
	TMap<UInstancedStaticMeshComponent*, FFoliageInstanceData> ReplaceableFoliagePool;

public:
	FORCEINLINE int GetSectionIndexX() const { return SectionIndexX; }
	FORCEINLINE int GetSectionIndexY() const { return SectionIndexY; }
	FORCEINLINE int GetLODLevel() const { return LODLevel; }
};

class FAsyncWorldGenerator : public FNonAbandonableTask
{
public:
	FAsyncWorldGenerator(AWorldEngine* InWorldGenerator) : WorldGenerator(InWorldGenerator) {}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncWorldGenerator, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork();

private:
	AWorldEngine* WorldGenerator;
};
