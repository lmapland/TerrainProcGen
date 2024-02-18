// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldEngine.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "FoliageType_InstancedStaticMesh.h"

AWorldEngine::AWorldEngine()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	TerrainMesh->bUseAsyncCooking = true;
	RootComponent = TerrainMesh;

	SeaComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeaComponent"));
	SeaComponent->SetupAttachment(GetRootComponent());
}

void AWorldEngine::RemoveFoliageTile2(const int TileIndex)
{
	if (!TerrainMesh->GetProcMeshSection(TileIndex)) return;

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
		//UE_LOG(LogTemp, Warning, TEXT("RemoveFoliageTile2(): FoliageComponents size: %i, In-loop (i: %i)"), FoliageComponents.Num(), i);

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

void AWorldEngine::BeginPlay()
{
	Super::BeginPlay();

	TileDiscardDistance = CellSize * (NumberSectionsX + NumberSectionsY) / 2 * (XVertexCount + YVertexCount) / 2;

	if (bRandomizeLayout)
	{
		PerlinOffset = FVector2D(FMath::FRandRange(-5000000.f, 5000000.f), FMath::FRandRange(-5000000.f, 5000000.f));
	}
}

void AWorldEngine::GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY, const int LODFactor)
{
	FVector2D LODVertexCount;
	LODVertexCount.X = XVertexCount / LODFactor;
	LODVertexCount.Y = YVertexCount / LODFactor;
	float LODCellSize = CellSize * LODFactor;
	float XGap = ((XVertexCount - 1) * CellSize - (LODVertexCount.X - 1) * LODCellSize) / LODCellSize;
	float YGap = ((YVertexCount - 1) * CellSize - (LODVertexCount.Y - 1) * LODCellSize) / LODCellSize;
	LODVertexCount.X += FMath::CeilToInt(XGap);
	LODVertexCount.Y += FMath::CeilToInt(YGap);

	FVector Offset = FVector();
	Offset.X = (XVertexCount - 1) * CellSize * InSectionIndexX;
	Offset.Y = (XVertexCount - 1) * CellSize * InSectionIndexY;

	TArray<FVector> Vertices;
	FVector Vertex;
	TArray<FVector2D> UVs;
	FVector2D UV;

	for (int y = -1; y <= LODVertexCount.Y; y++)
	{
		for (int x = -1; x <= LODVertexCount.X; x++)
		{
			Vertex.X = x * LODCellSize + Offset.X;
			Vertex.Y = y * LODCellSize + Offset.Y;
			Vertex.Z = GenHeight(FVector2D(Vertex.X, Vertex.Y));
			Vertices.Add(Vertex);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i (%f, %f, %f)"), y, x, Vertex.X, Vertex.Y, Vertex.Z);

			UV.X = ((LODVertexCount.X - 1) * InSectionIndexX + x) * (LODCellSize / 100.f);
			UV.Y = ((LODVertexCount.Y - 1) * InSectionIndexY + y) * (LODCellSize / 100.f);
			UVs.Add(UV);
		}
	}

	//if (Triangles.Num() == 0)
	//{
	Triangles.Empty();
	for (int y = 0; y <= LODVertexCount.Y; y++)
	{
		for (int x = 0; x <= LODVertexCount.X; x++)
		{
			int XYVertexCount = x + (y * (LODVertexCount.X + 2));
			Triangles.Add(XYVertexCount);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount);
			Triangles.Add(XYVertexCount + (LODVertexCount.X + 2));
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2));
			Triangles.Add(XYVertexCount + 1);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + 1);

			Triangles.Add(XYVertexCount + (LODVertexCount.X + 2));
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2) );
			Triangles.Add(XYVertexCount + (LODVertexCount.X + 2) + 1);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + (XVertexCount + 2) + 1);
			Triangles.Add(XYVertexCount + 1);
			//UE_LOG(LogTemp, Warning, TEXT("%i, %i: %i"), y, x, XYVertexCount + 1);
		}
	}
	//}

	GetNormalsTangents(Vertices, UVs, LODVertexCount, LODCellSize);
	//GetNormalsTangents(Vertices, Triangles, UVs, SubNormals, SubTangents, SubVertices, SubTriangles, SubUVs);
	bTileDataReady = true;
}

int AWorldEngine::DrawTile()
{
	//UE_LOG(LogTemp, Warning, TEXT("DrawTile()"));
	bTileDataReady = false;

	int DrawnMeshSection;
	int FurthestTileIndex = GetFurthestUpdatableTileIndex();
	TArray<FIntPoint> ValueArray;
	TArray<FIntPoint> KeyArray;

	if (FurthestTileIndex > -1)
	{
		// Found a tile that needs to be replaced
		QueuedTiles.GenerateKeyArray(KeyArray);
		QueuedTiles.GenerateValueArray(ValueArray);

		int ReplaceableMeshSection = ValueArray[FurthestTileIndex].X;
		DrawnMeshSection = ReplaceableMeshSection;
		RemoveFoliageTile2(ReplaceableMeshSection);

		TerrainMesh->ClearMeshSection(ReplaceableMeshSection);
		TerrainMesh->CreateMeshSection_LinearColor(ReplaceableMeshSection, SubVertices, SubTriangles, SubNormals, SubUVs, TArray<FLinearColor>(), SubTangents, true);

		QueuedTiles.Add(FIntPoint(SectionIndexX, SectionIndexY), FIntPoint(ReplaceableMeshSection, LODLevel));
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

	// Remove outdated LODs
	KeyArray.Empty();
	ValueArray.Empty();
	RemoveLODqueue.GenerateKeyArray(KeyArray);
	RemoveLODqueue.GenerateValueArray(ValueArray);
	if (RemoveLODqueue.Contains(FIntPoint(SectionIndexX, SectionIndexY)))
	{
		FIntPoint* Val = RemoveLODqueue.Find(FIntPoint(SectionIndexX, SectionIndexY));
		RemoveFoliageTile(Val->X);
		TerrainMesh->ClearMeshSection(Val->X);
		RemoveLODqueue.Remove(FIntPoint(SectionIndexX, SectionIndexY));
	}

	return DrawnMeshSection;
}

void AWorldEngine::GenerateTerrainAsync(const int InSectionIndexX, const int InSectionIndexY, const int InLODLevel)
{
	bGeneratorBusy = true;

	//UE_LOG(LogTemp, Warning, TEXT("GenerateTerrainAsync(): InLODLevel: %i"), InLODLevel);
	SectionIndexX = InSectionIndexX;
	SectionIndexY = InSectionIndexY;
	LODLevel = InLODLevel == 0 ? 1 : InLODLevel;

	//UE_LOG(LogTemp, Warning, TEXT("GenerateTerrainAsync(): LODLevel: %i"), LODLevel);
	QueuedTiles.Add(FIntPoint(SectionIndexX, SectionIndexY), FIntPoint(MeshSectionIndex, LODLevel));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]()
		{
			auto WorldGenTask = new FAsyncTask<FAsyncWorldGenerator>(this);
			WorldGenTask->StartBackgroundTask();
			WorldGenTask->EnsureCompletion();
			delete WorldGenTask;
		}
	);
}

int AWorldEngine::GenHeight(FVector2D Location)
{
	return GenerateHeightLayer(Location, 1 / ScaleLarge, AmplitudeLarge, FVector2D(.1f) + PerlinOffset) + GenerateHeightLayer(Location, 1 / ScaleMedium, AmplitudeMedium, FVector2D(.2f) + PerlinOffset);
}

int AWorldEngine::GenerateHeightLayer(const FVector2D Location, const float InScale, const float InAmplitude, const FVector2D Offset)
{
	float Result = FMath::PerlinNoise2D(Location * InScale + FVector2D(.1f, .1f) + Offset) * InAmplitude;
	return Result;
}

void AWorldEngine::GetNormalsTangents(TArray<FVector> InVertices, TArray<FVector2D> InUVs, FVector2D InLODVertexCount, float InLODCellSize)
{
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(InVertices, Triangles, InUVs, Normals, Tangents);

	int Index = 0;
	for (int y = -1; y <= InLODVertexCount.Y; y++)
	{
		for (int x = -1; x <= InLODVertexCount.X; x++)
		{
			if (y > -1 && y < InLODVertexCount.Y && x > -1 && x < InLODVertexCount.X)
			{
				SubVertices.Add(InVertices[Index]);
				SubUVs.Add(InUVs[Index]);
				SubNormals.Add(Normals[Index]);
				SubTangents.Add(Tangents[Index]);
			}
			Index++;
		}
	}

	//if (SubTriangles.Num() == 0)
	//{
	SubTriangles.Empty();
	for (int y = 0; y < InLODVertexCount.Y - 1; y++)
	{
		for (int x = 0; x < InLODVertexCount.X - 1; x++)
		{
			int XYVertexCount = y * InLODVertexCount.X + x;
			SubTriangles.Add(XYVertexCount);
			SubTriangles.Add(XYVertexCount + InLODVertexCount.X);
			SubTriangles.Add(XYVertexCount + 1);

			SubTriangles.Add(XYVertexCount + InLODVertexCount.X);
			SubTriangles.Add(XYVertexCount + InLODVertexCount.X + 1);
			SubTriangles.Add(XYVertexCount + 1);
		}
	}
	//}

	//UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i, %i, %i"), OutVertices.Num(), OutTriangles.Num(), OutNormals.Num(), OutUVs.Num(), OutTangents.Num());
}

FVector AWorldEngine::GetPlayerLocation()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	return Player ? Player->GetActorLocation() : FVector(0.f);
}

FVector2D AWorldEngine::GetTileLocation(FIntPoint TileCoordinate)
{
	return FVector2D(TileCoordinate * FIntPoint(XVertexCount - 1, YVertexCount - 1) * CellSize) + FVector2D(XVertexCount - 1, YVertexCount - 1) * CellSize / 2;
}

FIntPoint AWorldEngine::GetClosestQueuedTile()
{
	if (QueuedTiles.Num() == 0) return FIntPoint(0.f);

	float ClosestDistance = TNumericLimits<float>::Max();
	FIntPoint ClosestTile;

	for (const auto& Tile : QueuedTiles)
	{
		const FIntPoint& Key = Tile.Key;
		int Value = Tile.Value.X;
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

int AWorldEngine::GetFurthestUpdatableTileIndex()
{
	if (QueuedTiles.Num() == 0) return -1;

	float FurthestDistance = -1.f;
	int FurthestTileIndex = -1;
	int CurrentIndex = 0;

	for (const auto& Tile : QueuedTiles)
	{
		const FIntPoint& Key = Tile.Key;
		int Value = Tile.Value.X;
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

void AWorldEngine::AddFoliageInstances(const FVector InLocation)
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

void AWorldEngine::SpawnFoliageCluster(UFoliageType_InstancedStaticMesh* FoliageType, UInstancedStaticMeshComponent* FoliageIsmComponent, const FVector ClusterLocation)
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

void AWorldEngine::QueueTileForGeneration(const int InSectionX, const int InSectionY, const int InMeshSectionIndex)
{
	// Don't generate tile if tile is already outside the player's DiscardDistance
	FVector PlayerLocation = GetPlayerLocation();
	float Distance = FVector2D::Distance(FVector2D(PlayerLocation.X, PlayerLocation.Y), GetTileLocation(FIntPoint(InSectionX, InSectionY)));

	if (Distance > TileDiscardDistance)
	{
		return;
	}

	// Get current LOD
	float XLoc = PlayerLocation.X / ((XVertexCount - 1) * CellSize);
	float YLoc = PlayerLocation.Y / ((YVertexCount - 1) * CellSize);
	Distance = FVector2D::Distance(FVector2D(InSectionX + 0.5f, InSectionY + 0.5f), FVector2D(XLoc, YLoc));
	int LODToUse = FMath::TruncToInt(FMath::Max(1.f, Distance));
	FIntPoint SectionPoint = FIntPoint(InSectionX, InSectionY);

	// Check if the given SectionX and Y are actually in the QueuedTiles TMap
	// If yes, check their LOD level and make sure we don't need to adjust the LOD level
	// If no, add this tile to the QueuedTiles TMap
	FIntPoint* FoundTile = QueuedTiles.Find(SectionPoint);
	if (FoundTile == nullptr)
	{
		QueuedTiles.Add(SectionPoint, FIntPoint(-1, LODToUse));
	}
	else if (FoundTile->Y != LODToUse) // Tile exists; check data to decide what to do with it
	{
		if (FoundTile->X != -1)
		{
			// Need to remove the current Tile and add this Tile to be queued so it can be generated with the correct LOD
			RemoveLODqueue.Add(SectionPoint, *FoundTile);
		}

		QueuedTiles.Add(SectionPoint, FIntPoint(-1, LODToUse));
	}
}

void FAsyncWorldGenerator::DoWork()
{
	WorldGenerator->GenerateTerrain(WorldGenerator->GetSectionIndexX(), WorldGenerator->GetSectionIndexY(), WorldGenerator->GetLODLevel());
}
