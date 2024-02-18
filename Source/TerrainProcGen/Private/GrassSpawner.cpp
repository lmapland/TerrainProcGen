// Fill out your copyright notice in the Description page of Project Settings.

#include "GrassSpawner.h"
#include "FoliageType_InstancedStaticMesh.h"

void AGrassSpawner::SpawnObject(const FHitResult& Hit, const FVector TileCenter)
{
	if (Hit.Location.Z < -100.f)
	{
		return;
	}

	if (Hit.PhysMaterial == nullptr || Hit.PhysMaterial->SurfaceType != SupportedSurfaceType)
	{
		return;
	}

	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		UFoliageType_InstancedStaticMesh* FoliageType = FoliageTypes[i];

		// Check Z
		if (Hit.Location.Z < FoliageType->Height.Min || Hit.Location.Z > FoliageType->Height.Max)
		{
			continue;
		}

		if (FoliageType->InitialSeedDensity < GrassSeed.FRandRange(0.f, 10.f))
		{
			continue;
		}

		// Check ground slope
		float DotProduct = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
		float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (SlopeAngle < FoliageType->GroundSlopeAngle.Min || SlopeAngle > FoliageType->GroundSlopeAngle.Max)
		{
			continue;
		}

		FTransform InstanceTransform = FTransform();
		InstanceTransform.SetLocation(Hit.Location);

		if (FoliageType->AlignToNormal)
		{
			InstanceTransform.SetRotation(FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator().Quaternion());
		}
		else
		{
			InstanceTransform.SetRotation(FRotator(0, FMath::RandRange(0, 360), 0).Quaternion());
		}

		InstanceTransform.SetScale3D(FVector(FMath::RandRange(FoliageType->ProceduralScale.Min, FoliageType->ProceduralScale.Max)));
		FoliageComponents[i]->AddInstance(InstanceTransform, true);
	}
}

void AGrassSpawner::RemoveSubTiles(const FVector TileCenter)
{
	Super::RemoveSubTiles(TileCenter);

	FVector Min = FVector(TileCenter + FVector::One() * CellSize * -0.5f);
	FVector Max = FVector(TileCenter + FVector::One() * CellSize * 0.5f);
	Min.Z = TileCenter.Z - TraceDistance;
	Max.Z = TileCenter.Z + TraceDistance;
	FBox Box = FBox(Min, Max);

	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		TArray<int> GrassToRemove = FoliageComponents[i]->GetInstancesOverlappingBox(Box, true);

		if (GrassToRemove.Num() > 0)
		{
			FoliageComponents[i]->RemoveInstances(GrassToRemove);
		}
	}
}
