// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "FoliageType_InstancedStaticMesh.h"

//#include "Debug/DebugDrawComponent.h"

AWorldGenerator::AWorldGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	TerrainMesh->bUseAsyncCooking = true;
	RootComponent = TerrainMesh;

}

void AWorldGenerator::RemoveFoliageTile2(const int TileIndex)
{
	TArray<FProcMeshVertex> Vertices = TerrainMesh->GetProcMeshSection(TileIndex)->ProcVertexBuffer;

	if (Vertices.Num() == 0)
	{
		return;
	}

	FVector FirstVertex = Vertices[0].Position + GetActorLocation();
	FVector LastVertex = Vertices[Vertices.Num() - 1].Position + GetActorLocation();
	float MaxHeight = AmplitudeLarge + AmplitudeMedium + AmplitudeSmall;
	FBox Box = FBox(FVector(FirstVertex.X, FirstVertex.Y, MaxHeight * -1), FVector(LastVertex.X, LastVertex.Y, MaxHeight));

	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveFoliageTile2(): FoliageComponents size: %i, In-loop (i: %i)"), FoliageComponents.Num(), i);

		TArray<int> FoliageToRemove = FoliageComponents[i]->GetInstancesOverlappingBox(Box);

		FFoliageInstanceData* FoliageData = ReplaceableFoliagePool.Find(FoliageComponents[i]);
		if (FoliageData)
		{
			FoliageData->Instances.Append(FoliageToRemove);
		}
		else
		{
			ReplaceableFoliagePool.Add(FoliageComponents[i], FFoliageInstanceData(FoliageToRemove));
		}
	}

	return;
}

void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();

	TileDiscardDistance = CellSize * (NumberSectionsX + NumberSectionsY) / 2 * (XVertexCount + YVertexCount) / 2;

	//UE_LOG(LogTemp, Warning, TEXT("TileDiscardDistance: %f"), TileDiscardDistance);

	if (bRandomizeLayout)
	{
		PerlinOffset = FVector2D(FMath::FRandRange(-5000000.f, 5000000.f), FMath::FRandRange(-5000000.f, 5000000.f));
		//AmplitudeLarge = AmplitudeLarge * FMath::FRandRange(.5f, 2.f);
		//AmplitudeMedium = AmplitudeMedium * FMath::FRandRange(.5f, 2.f);
	}
}

void AWorldGenerator::GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY)
{
	FVector Offset = FVector();
	Offset.X = (XVertexCount - 1) * CellSize * InSectionIndexX;
	Offset.Y = (YVertexCount - 1) * CellSize * InSectionIndexY;

	TArray<FVector> Vertices;
	FVector Vertex;
	TArray<FVector2D> UVs;
	FVector2D UV;

	for (int y = -1; y <= YVertexCount; y++)
	{
		for (int x = -1; x <= XVertexCount; x++)
		{
			Vertex.X = x * CellSize + Offset.X;
			Vertex.Y = y * CellSize + Offset.Y;
			Vertex.Z = GenHeight(FVector2D(Vertex.X, Vertex.Y));
			Vertices.Add(Vertex);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i (%f, %f, %f)"), y, x, Vertex.X, Vertex.Y, Vertex.Z);
			//DrawDebugSphere(GetWorld(), Vertex, 5.f, 8, FColor::Red, true, 120.f);

			UV.X = ((XVertexCount - 1) * InSectionIndexX + x) * (CellSize / 100.f);
			UV.Y = ((YVertexCount - 1) * InSectionIndexY + y) * (CellSize / 100.f);
			UVs.Add(UV);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i (%f, %f)"), y, x, UV.X, UV.Y);
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("GenerateTerrain(): after first for loop"));
	if (Triangles.Num() == 0)
	{
		for (int y = 0; y <= YVertexCount; y++)
		{
			for (int x = 0; x <= XVertexCount; x++)
			{
				int XYVertexCount = x + (y * (XVertexCount + 2));
				Triangles.Add(XYVertexCount);
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount);
				Triangles.Add(XYVertexCount + (XVertexCount + 2));
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2));
				Triangles.Add(XYVertexCount + 1);
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + 1);

				Triangles.Add(XYVertexCount + (XVertexCount + 2));
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2) );
				Triangles.Add(XYVertexCount + (XVertexCount + 2) + 1);
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2) + 1);
				Triangles.Add(XYVertexCount + 1);
				//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + 1);
			}
		}
	}

	GetNormalsTangents(Vertices, Triangles, UVs, SubNormals, SubTangents, SubVertices, SubTriangles, SubUVs);
	bTileDataReady = true;
}

int AWorldGenerator::DrawTile()
{
	//UE_LOG(LogTemp, Warning, TEXT("DrawTile()"));
	bTileDataReady = false;

	int DrawnMeshSection;

	int FurthestTileIndex = GetFurthestUpdatableTileIndex();

	if (FurthestTileIndex > -1)
	{
		// Found a tile that needs to be replaced
		TArray<int> ValueArray;
		TArray<FIntPoint> KeyArray;
		QueuedTiles.GenerateKeyArray(KeyArray);
		QueuedTiles.GenerateValueArray(ValueArray);

		int ReplaceableMeshSection = ValueArray[FurthestTileIndex];
		DrawnMeshSection = ReplaceableMeshSection;
		RemoveFoliageTile2(ReplaceableMeshSection);

		TerrainMesh->ClearMeshSection(ReplaceableMeshSection);
		TerrainMesh->CreateMeshSection_LinearColor(ReplaceableMeshSection, SubVertices, SubTriangles, SubNormals, SubUVs, TArray<FLinearColor>(), SubTangents, true);

		QueuedTiles.Add(FIntPoint(SectionIndexX, SectionIndexY), ReplaceableMeshSection);
		QueuedTiles.Remove(KeyArray[FurthestTileIndex]);
	}
	else
	{
		TerrainMesh->CreateMeshSection_LinearColor(MeshSectionIndex, SubVertices, SubTriangles, SubNormals, SubUVs, TArray<FLinearColor>(), SubTangents, true);
		if (TerrainMaterial)
		{
			TerrainMesh->SetMaterial(MeshSectionIndex, TerrainMaterial);
		}
		DrawnMeshSection = MeshSectionIndex;
		MeshSectionIndex++;
	}

	SubVertices.Empty();
	SubNormals.Empty();
	SubTangents.Empty();
	SubUVs.Empty();

	return DrawnMeshSection;
}

void AWorldGenerator::GenerateTerrainAsync(const int InSectionIndexX, const int InSectionIndexY)
{
	bGeneratorBusy = true;

	SectionIndexX = InSectionIndexX;
	SectionIndexY = InSectionIndexY;
	QueuedTiles.Add(FIntPoint(SectionIndexX, SectionIndexY), MeshSectionIndex);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]()
		{
			auto WorldGenTask = new FAsyncTask<FAsyncWorldGenerator>(this);
			WorldGenTask->StartBackgroundTask();
			WorldGenTask->EnsureCompletion();
			delete WorldGenTask;
		}
	);
}

int AWorldGenerator::GenHeight(FVector2D Location)
{
	return GenerateHeightLayer(Location, 1 / ScaleLarge, AmplitudeLarge, FVector2D(.1f) + PerlinOffset) + GenerateHeightLayer(Location, 1 / ScaleMedium, AmplitudeMedium, FVector2D(.2f) + PerlinOffset);
}

int AWorldGenerator::GenerateHeightLayer(const FVector2D Location, const float InScale, const float InAmplitude, const FVector2D Offset)
{
	float Result2 = FMath::PerlinNoise2D(Location * InScale + FVector2D(.1f, .1f) + Offset) * InAmplitude;
	return Result2;

	//UE_LOG(LogTemp, Warning, TEXT("PerlinNoise2D: InValue: (%f, %f); Perlin Result: %f"), Perlin2.X, Perlin2.Y, Result);

	/*float X = Location.X * InScale * Offset.X;
	float Y = Location.Y * InScale * Offset.Y;
	UE_LOG(LogTemp, Warning, TEXT("PerlinNoise1D: NoiseX: %f, NoiseY: %f"), X, Y);

	float NoiseX = FMath::PerlinNoise1D(X);
	float NoiseY = FMath::PerlinNoise1D(Y);

	float NoiseResult = NoiseX * NoiseY * InAmplitude;
	UE_LOG(LogTemp, Warning, TEXT("PerlinNoise1D: NoiseX: %f, NoiseY: %f, Final Result: %f"), NoiseX, NoiseY, NoiseResult);
	return NoiseResult;*/
}

void AWorldGenerator::GetNormalsTangents(TArray<FVector> InVertices, TArray<int> InTriangles, TArray<FVector2D> InUVs, TArray<FVector>& OutNormals, TArray<FProcMeshTangent>& OutTangents, TArray<FVector>& OutVertices, TArray<int>& OutTriangles, TArray<FVector2D>& OutUVs)
{
	//UE_LOG(LogTemp, Warning, TEXT("In GetNormalsTangents()"));
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(InVertices, InTriangles, InUVs, Normals, Tangents);

	int Index = 0;
	//UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i, %i"), InVertices.Num(), InUVs.Num(), Normals.Num(), Tangents.Num());
	for (int y = -1; y <= YVertexCount; y++)
	{
		for (int x = -1; x <= XVertexCount; x++)
		{
			if (y > -1 && y < YVertexCount && x > -1 && x < XVertexCount)
			{
				OutVertices.Add(InVertices[Index]);
				OutUVs.Add(InUVs[Index]);
				OutNormals.Add(Normals[Index]);
				OutTangents.Add(Tangents[Index]);
			}
			Index++;
		}
	}

	if (OutTriangles.Num() == 0)
	{
		for (int y = 0; y < YVertexCount - 1; y++)
		{
			for (int x = 0; x < XVertexCount - 1; x++)
			{
				int XYVertexCount = y * XVertexCount + x;
				OutTriangles.Add(XYVertexCount);
				OutTriangles.Add(XYVertexCount + XVertexCount);
				OutTriangles.Add(XYVertexCount + 1);

				OutTriangles.Add(XYVertexCount + XVertexCount);
				OutTriangles.Add(XYVertexCount + XVertexCount + 1);
				OutTriangles.Add(XYVertexCount + 1);
			}
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i, %i, %i"), OutVertices.Num(), OutTriangles.Num(), OutNormals.Num(), OutUVs.Num(), OutTangents.Num());
}

FVector AWorldGenerator::GetPlayerLocation()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	return Player ? Player->GetActorLocation() : FVector(0.f);
}

FVector2D AWorldGenerator::GetTileLocation(FIntPoint TileCoordinate)
{
	return FVector2D(TileCoordinate * FIntPoint(XVertexCount - 1, YVertexCount - 1) * CellSize) + FVector2D(XVertexCount - 1, YVertexCount - 1) * CellSize / 2;
}

FIntPoint AWorldGenerator::GetClosestQueuedTile()
{
	if (QueuedTiles.Num() == 0) return FIntPoint(0.f);

	float ClosestDistance = TNumericLimits<float>::Max();
	FIntPoint ClosestTile;

	for (const auto& Tile : QueuedTiles)
	{
		const FIntPoint& Key = Tile.Key;
		int Value = Tile.Value;
		if (Value == -1) // Find only unrendered tiles
		{
			FVector2D TileLocation = GetTileLocation(Key);
			FVector PlayerLocation = GetPlayerLocation();
			float Distance = FVector2D::Distance(TileLocation, FVector2D(PlayerLocation));

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestTile = Key;
			}
		}
	}
	return ClosestTile;
}

int AWorldGenerator::GetFurthestUpdatableTileIndex()
{
	if (QueuedTiles.Num() == 0) return -1;

	float FurthestDistance = -1.f;
	int FurthestTileIndex = -1;
	int CurrentIndex = 0;

	for (const auto& Tile : QueuedTiles)
	{
		const FIntPoint& Key = Tile.Key;
		int Value = Tile.Value;
		if (Value != -1) // Find only rendered tiles
		{
			FVector2D TileLocation = GetTileLocation(Key);
			FVector PlayerLocation = GetPlayerLocation();
			float Distance = FVector2D::Distance(TileLocation, FVector2D(PlayerLocation));

			if (Distance > FurthestDistance && Distance > TileDiscardDistance)
			{
				FurthestDistance = Distance;
				FurthestTileIndex = CurrentIndex;
			}
		}
		CurrentIndex++;
	}
	return FurthestTileIndex;
}

void AWorldGenerator::AddFoliageInstances(const FVector InLocation)
{
	for (int FoliageTypeIndex = 0; FoliageTypeIndex < FoliageTypes.Num(); FoliageTypeIndex++)
	{
		UFoliageType_InstancedStaticMesh* FoliageType = FoliageTypes[FoliageTypeIndex];
		if (!FoliageType)
		{
			continue;
		}

		if (InLocation.Z < FoliageType->Height.Min || InLocation.Z > FoliageType->Height.Max)
		{
			continue;
		}

		if (FoliageType->InitialSeedDensity < FoliageSeed.FRandRange(0.f, 10.f))
		{
			continue;
		}

		UInstancedStaticMeshComponent* FoliageInstance = FoliageComponents[FoliageTypeIndex];
		SpawnFoliageCluster(FoliageType, FoliageInstance, InLocation);
	}
}

void AWorldGenerator::SpawnFoliageCluster(UFoliageType_InstancedStaticMesh* FoliageType, UInstancedStaticMeshComponent* FoliageIsmComponent, const FVector ClusterLocation)
{
	int MaxSteps = FoliageSeed.RandRange(0, FoliageType->NumSteps);
	FVector ClusterBase = ClusterLocation;

	for (int i = 0; i < MaxSteps; i++)
	{
		ClusterBase += FoliageSeed.GetUnitVector() * FoliageType->AverageSpreadDistance;

		FVector InstanceLocation = ClusterBase + FoliageSeed.GetUnitVector() * FoliageType->SpreadVariance;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, InstanceLocation + FVector(0, 0, 2000), InstanceLocation + FVector(0, 0, -2000), ECC_WorldStatic, CollisionParams);

		if (!bHit || HitResult.Component != TerrainMesh)
		{
			continue;
		}

		// Check ground slope
		float DotProduct = FVector::DotProduct(HitResult.ImpactNormal, FVector::UpVector);
		float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (SlopeAngle < FoliageType->GroundSlopeAngle.Min || SlopeAngle > FoliageType->GroundSlopeAngle.Max)
		{
			continue;
		}

		// Check Z
		if (HitResult.Location.Z < FoliageType->Height.Min || HitResult.Location.Z > FoliageType->Height.Max)
		{
			continue;
		}

		FTransform InstanceTransform = FTransform();
		InstanceTransform.SetLocation(HitResult.Location + FVector(0, 0, FoliageSeed.FRandRange(FoliageType->ZOffset.Min, FoliageType->ZOffset.Max)));
		InstanceTransform.SetScale3D(FVector::One() * FoliageSeed.FRandRange(FoliageType->ProceduralScale.Min, FoliageType->ProceduralScale.Max));
		if (FoliageType->RandomYaw)
		{
			InstanceTransform.SetRotation(FRotator(0, FoliageSeed.FRandRange(0, 360), 0).Quaternion());
		}

		// Relocate foliage instances from pool
		FFoliageInstanceData* FoliageData = ReplaceableFoliagePool.Find(FoliageIsmComponent);
		if (FoliageData && FoliageData->Instances.Num() > 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("SpawnFoliageCluster(): Updating existing foliage data"));
			FoliageIsmComponent->UpdateInstanceTransform(FoliageData->Instances[FoliageData->Instances.Num() - 1], InstanceTransform);
			FoliageData->Instances.RemoveAt(FoliageData->Instances.Num() - 1);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("SpawnFoliageCluster(): Adding new foliage instance"));
			FoliageIsmComponent->AddInstance(InstanceTransform, true);
		}
	}
}

//void AWorldGenerator::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void FAsyncWorldGenerator::DoWork()
{
	WorldGenerator->GenerateTerrain(WorldGenerator->GetSectionIndexX(), WorldGenerator->GetSectionIndexY());
}
