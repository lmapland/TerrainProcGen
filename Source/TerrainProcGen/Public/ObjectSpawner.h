// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawner.generated.h"

UCLASS()
class TERRAINPROCGEN_API AObjectSpawner : public AActor
{
	GENERATED_BODY()
	
public:
	AObjectSpawner();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FVector GetPlayerCell();

	UFUNCTION(BlueprintCallable)
	void UpdateTiles();

	UFUNCTION(BlueprintCallable)
	void GenerateSubTiles(const FVector TileCenter);

	UFUNCTION(BlueprintCallable)
	void RemoveFarTiles();

	UFUNCTION(BlueprintCallable)
	virtual void RemoveSubTiles(const FVector TileCenter);

	UFUNCTION(BlueprintCallable)
	virtual void SpawnObject(const FHitResult& Hit);

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Spawner|Defaults")
	int CellSize = 5000;

	/* How to divide up the CellSize. If CellSize == 5000 and CellDivisions == 10, then SubCellSize == 500 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Spawner|Defaults")
	int CellDivisions = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Spawner|Defaults")
	int CellCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Spawner|Defaults")
	int TraceDistance = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Spawner|Defaults")
	int SubCellRandomOffset = 200;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Spawner|Defaults")
	TArray<FVector2D> ActiveTiles;
};
