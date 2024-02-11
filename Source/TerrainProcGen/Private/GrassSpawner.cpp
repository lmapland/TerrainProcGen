// Fill out your copyright notice in the Description page of Project Settings.

#include "GrassSpawner.h"
#include "FoliageType_InstancedStaticMesh.h"

void AGrassSpawner::SpawnObject(const FHitResult& Hit)
{
	if (Hit.PhysMaterial == nullptr || Hit.PhysMaterial->SurfaceType != SupportedSurfaceType)
	{
		return;
	}

	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		FTransform InstanceTransform = FTransform();
		InstanceTransform.SetLocation(Hit.Location);
		InstanceTransform.SetRotation(FRotator(0, FMath::RandRange(0, 360), 0).Quaternion());
		InstanceTransform.SetScale3D(FVector(FMath::RandRange(GrassSizeMin, GrassSizeMax)));
		FoliageComponents[i]->AddInstance(InstanceTransform, true);
	}
}
