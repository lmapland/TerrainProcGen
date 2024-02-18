// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimalSpawner.h"

void AAnimalSpawner::SpawnObject(const FHitResult& Hit, const FVector TileCenter)
{
	if (FMath::RandRange(0, 100) > AnimalSpawnPercentage)
	{
		return;
	}

	//DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + FVector::UpVector * 10000, FColor::Red, false, 60.f, 0, 1.f);
	if (Hit.Location.Z < -100.f)
	{
		return;
	}

	if (Hit.PhysMaterial == nullptr || Hit.PhysMaterial->SurfaceType != SupportedSurfaceType)
	{
		return;
	}

	for (int i = 0; i < AnimalTypes.Num(); i++)
	{
		AActor* Animal = GetAnimalFromPool(AnimalTypes[i].AnimalClassRef);
		if (Animal)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Getting Animal from the pool"));
			Animal->SetActorLocation(Hit.Location + FVector(0.f, 0.f, 1000.f));
			Animal->SetActorRotation(FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f));
			DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + FVector::UpVector * 10000, FColor::Blue, false, 60.f, 0, 1.f);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Creating new Animal"));
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			SpawnParameters.bNoFail = true;
 
			Animal = GetWorld()->SpawnActor<AActor>(AnimalTypes[i].AnimalClassRef, Hit.Location + FVector(0.f, 0.f, 1000.f), FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f), SpawnParameters);
		}

		if (!Animal) continue;

		FAnimalData* AnimalData = AnimalsOfTile.Find(TileCenter);
		if (AnimalData)
		{
			AnimalData->SpawnedAnimals.Add(Animal);
		}
		else
		{
			AnimalsOfTile.Add(TileCenter, FAnimalData({ Animal }));
		}
	}
}

void AAnimalSpawner::RemoveSubTiles(const FVector TileCenter)
{
	FAnimalData* AnimalData = AnimalsOfTile.Find(TileCenter);
	if (!AnimalData)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Destroying Animals from Tile %f, %f"), TileCenter.X, TileCenter.Y);

	for (int i = 0; i < AnimalData->SpawnedAnimals.Num(); i++)
	{
		FAnimalData* PooledAnimalData = PooledAnimals.Find(AnimalData->SpawnedAnimals[i]->GetClass());
		if (PooledAnimalData)
		{
			PooledAnimalData->SpawnedAnimals.Add(AnimalData->SpawnedAnimals[i]);
		}
		else
		{
			PooledAnimals.Add(AnimalData->SpawnedAnimals[i]->GetClass(), FAnimalData({AnimalData->SpawnedAnimals[i]}));
		}
	}
	AnimalData->SpawnedAnimals.Empty();
}

AActor* AAnimalSpawner::GetAnimalFromPool(const TSubclassOf<AActor> AnimalClass)
{
	AActor* Animal = nullptr;
	FAnimalData* AnimalData = PooledAnimals.Find(AnimalClass);
	if (AnimalData && AnimalData->SpawnedAnimals.Num() > 0)
	{
		Animal = AnimalData->SpawnedAnimals.Pop();
	}

	return Animal;
}
