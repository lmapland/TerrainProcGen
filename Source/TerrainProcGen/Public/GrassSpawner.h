// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectSpawner.h"
#include "GrassSpawner.generated.h"

class UFoliageType_InstancedStaticMesh;

UCLASS()
class TERRAINPROCGEN_API AGrassSpawner : public AObjectSpawner
{
	GENERATED_BODY()

public:
	void SpawnObject(const FHitResult& Hit, const FVector TileCenter) override;
	void RemoveSubTiles(const FVector TileCenter) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Spawner|Defaults")
	TArray<UInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Spawner|Defaults")
	TArray<UFoliageType_InstancedStaticMesh*> FoliageTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Spawner|Defaults")
	TEnumAsByte<EPhysicalSurface> SupportedSurfaceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Spawner|Defaults")
	FRandomStream GrassSeed;

};
