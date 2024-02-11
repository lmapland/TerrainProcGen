// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectSpawner.h"

AObjectSpawner::AObjectSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 3.f;
}

void AObjectSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTiles();
}

FVector AObjectSpawner::GetPlayerCell()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AActor* Player = PC->GetPawn();
	if (!Player)
	{
		return FVector::Zero();
	}

	FVector Location = PC->GetPawn()->GetActorLocation();

	Location = Location / CellSize;
	Location = FVector(FMath::RoundToInt(Location.X), FMath::RoundToInt(Location.Y), FMath::RoundToInt(Location.Z)) * CellSize;

	return Location;
}

void AObjectSpawner::UpdateTiles()
{
	FVector Origin = GetPlayerCell();
	for (int y = CellCount * -.5f; y <= CellCount * .5f; y++)
	{
		for (int x = CellCount * -.5f; x <= CellCount * .5f; x++)
		{
			FVector TileCenter = Origin + FVector(x, y, 0.f) * CellSize;

			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(GetWorld()->GetFirstPlayerController()->GetPawn());
			GetWorld()->LineTraceSingleByChannel(HitResult, TileCenter + FVector::UpVector * TraceDistance, TileCenter - FVector::UpVector * TraceDistance, ECC_Visibility, CollisionParams);

			if (HitResult.bBlockingHit)
			{
				if (!ActiveTiles.Contains(FVector2D(TileCenter.X, TileCenter.Y)))
				{
					ActiveTiles.Add(FVector2D(TileCenter.X, TileCenter.Y));
					GenerateSubTiles(TileCenter);
					DrawDebugBox(GetWorld(), HitResult.Location, FVector(CellSize * .5f), FColor::Emerald, false, 8);
				}
			}

		}
	}
}

void AObjectSpawner::GenerateSubTiles(const FVector TileCenter) // :UpdateTile(
{
	int SubCellSize = CellSize / CellDivisions;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetWorld()->GetFirstPlayerController()->GetPawn());
	CollisionParams.bReturnPhysicalMaterial = true;

	for (int y = CellSize * -.5f; y < CellSize * .5f; y += SubCellSize)
	{
		for (int x = CellSize * -.5f; x < CellSize * .5f; x += SubCellSize)
		{
			FVector SubCellLocation = TileCenter + FVector(x + FMath::RandRange(-SubCellRandomOffset, SubCellRandomOffset), y + FMath::RandRange(-SubCellRandomOffset, SubCellRandomOffset), 0);
			GetWorld()->LineTraceSingleByChannel(HitResult, SubCellLocation + FVector::UpVector * TraceDistance, SubCellLocation - FVector::UpVector * TraceDistance, ECC_WorldStatic, CollisionParams);

			if (HitResult.bBlockingHit)
			{
				SpawnObject(HitResult);
			}
		}
	}
}

void AObjectSpawner::SpawnObject(const FHitResult& Hit)
{
}
