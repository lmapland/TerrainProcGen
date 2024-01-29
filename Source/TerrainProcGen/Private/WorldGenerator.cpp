// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

//#include "Debug/DebugDrawComponent.h"

AWorldGenerator::AWorldGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
}

void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
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
	TArray<int> Triangles;

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

	//UE_LOG(LogTemp, Warning, TEXT("GenerateTerrain(): after second for loop"));

	TArray<FVector> SubNormals;
	TArray<FProcMeshTangent> SubTangents;
	TArray<FVector2D> SubUVs;
	TArray<int> SubTriangles;
	TArray<FVector> SubVertices;
	GetNormalsTangents(Vertices, Triangles, UVs, SubNormals, SubTangents, SubVertices, SubTriangles, SubUVs);
	//UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i, %i, %i"), SubNormals.Num(), SubTangents.Num(), SubUVs.Num(), SubTriangles.Num(), SubVertices.Num());
	TerrainMesh->CreateMeshSection_LinearColor(MeshSectionIndex, SubVertices, SubTriangles, SubNormals, SubUVs, TArray<FLinearColor>(), SubTangents, true);
	if (TerrainMaterial)
	{
		TerrainMesh->SetMaterial(MeshSectionIndex, TerrainMaterial);
	}
	MeshSectionIndex++;
}

int AWorldGenerator::GenHeight(FVector2D Location)
{
	return GenerateHeightLayer(Location, ScaleLarge, AmplitudeLarge, FVector2D(.1f)); // + GenerateHeightLayer(Location, ScaleMedium, AmplitudeMedium, FVector2D(.2f)) + GenerateHeightLayer(Location, ScaleSmall, AmplitudeSmall, FVector2D(.4f));
}

int AWorldGenerator::GenerateHeightLayer(const FVector2D Location, const float InScale, const float InAmplitude, const FVector2D Offset)
{
	float Result2 = FMath::PerlinNoise2D(Location * InScale + FVector2D(.1f, .1f) + Offset) * InAmplitude;
	FVector2D Perlin1 = Location * InScale + FVector2D(.1f, .1f);
	FVector2D Perlin2 = Perlin1 + Offset;
	float Result = FMath::PerlinNoise2D(Perlin2) * InAmplitude;
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
	TArray<int> Triangles;
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

	//UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i, %i, %i"), OutVertices.Num(), OutTriangles.Num(), OutNormals.Num(), OutUVs.Num(), OutTangents.Num());
}

//void AWorldGenerator::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

