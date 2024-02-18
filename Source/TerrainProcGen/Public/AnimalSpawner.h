// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectSpawner.h"
#include "AnimalSpawner.generated.h"


USTRUCT(BlueprintType)
struct FAnimalType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animals")
	TSubclassOf<AActor> AnimalClassRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animals")
	int HerdSize = 1;
};

USTRUCT(BlueprintType)
struct FAnimalData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animals")
	TArray<AActor*> SpawnedAnimals;
};

UCLASS()
class TERRAINPROCGEN_API AAnimalSpawner : public AObjectSpawner
{
	GENERATED_BODY()
public:
	virtual void SpawnObject(const FHitResult& Hit, const FVector TileCenter) override;
	virtual void RemoveSubTiles(const FVector TileCenter) override;

protected:
	AActor* GetAnimalFromPool(const TSubclassOf<AActor> AnimalClass);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animals")
	TArray<FAnimalType> AnimalTypes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animals")
	int AnimalSpawnPercentage = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animals")
	TEnumAsByte<EPhysicalSurface> SupportedSurfaceType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animals")
	TMap<FVector, FAnimalData> AnimalsOfTile;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animals")
	TMap<TSubclassOf<AActor>, FAnimalData> PooledAnimals;
	
};
