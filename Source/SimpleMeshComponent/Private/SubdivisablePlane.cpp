// Copyright Epic Games, Inc. All Rights Reserved.
//---------EXAMPLE MESH---------//
// Licence: MIT License
// Created by: BenjaCorp at iolaCorpStudio
// Created at: 20/02/2024
//---------EXAMPLE MESH---------//

#include "SubdivisablePlane.h"
#include "SimpleMeshComponent.h"
#include "DynamicMeshBuilder.h"


ASubdivisablePlane::ASubdivisablePlane()
{
    PrimaryActorTick.bCanEverTick = true;
    

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
    Root->SetMobility(EComponentMobility::Static);
    SimpleMeshComponent = CreateDefaultSubobject<USimpleMeshComponent>(TEXT("SimpleMeshComponent"));
    SimpleMeshComponent->SetupAttachment(Root);
    SimpleMeshComponent->SetMobility(EComponentMobility::Static);
    
    // Enable collision
    SimpleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SimpleMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    UMaterialInterface* Material1 = MaterialSlot1;
    UMaterialInterface* Material2 = MaterialSlot2;

   
        Material1 = LoadObject<UMaterialInterface>(nullptr, TEXT("MaterialInterface'/VoxelScape/Materials/CubesTypes/MI_Grass.MI_Grass'"));
   
        Material2 = LoadObject<UMaterialInterface>(nullptr, TEXT("MaterialInterface'/VoxelScape/Materials/CubesTypes/MI_Dirt.MI_Dirt'"));
    
}

void ASubdivisablePlane::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    InitializePlaneGeometry();
}

void ASubdivisablePlane::BeginPlay()
{
    Super::BeginPlay();
    InitializePlaneGeometry();
}

void ASubdivisablePlane::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASubdivisablePlane::InitializePlaneGeometry()
{
    double StartTime = FPlatformTime::Seconds();
    if (NumDivisionsX < 1 || NumDivisionsY < 1) return;

    TArray<FVector> Vertices; // Modification pour utiliser FVector directement
    TArray<int32> Indices;

    float SizeX = 100.0f;
    float SizeY = 100.0f;
    float DeltaX = SizeX / NumDivisionsX;
    float DeltaY = SizeY / NumDivisionsY;

    // Générer les sommets
    for (int32 y = 0; y <= NumDivisionsY; ++y)
    {
        for (int32 x = 0; x <= NumDivisionsX; ++x)
        {
            FVector Position(x * DeltaX, y * DeltaY, 0.0f);
            Vertices.Add(Position); // Plus besoin de convertir ici
        }
    }

    // Générer les indices pour chaque quad
    for (int32 y = 0; y < NumDivisionsY; ++y)
    {
        for (int32 x = 0; x < NumDivisionsX; ++x)
        {
            int32 Index = x + (NumDivisionsX + 1) * y;
            Indices.Add(Index);
            Indices.Add(Index + NumDivisionsX + 1);
            Indices.Add(Index + 1);

            Indices.Add(Index + 1);
            Indices.Add(Index + NumDivisionsX + 1);
            Indices.Add(Index + NumDivisionsX + 2);
        }
    }

    UMaterialInterface* Material1 = MaterialSlot1;
    UMaterialInterface* Material2 = MaterialSlot2;

    if (!Material1)
    {
        Material1 = LoadObject<UMaterialInterface>(nullptr, TEXT("MaterialInterface'/VoxelScape/Materials/CubesTypes/MI_Grass.MI_Grass'"));
    }
    if (!Material1)
    {
        Material2 = LoadObject<UMaterialInterface>(nullptr, TEXT("MaterialInterface'/VoxelScape/Materials/CubesTypes/MI_Dirt.MI_Dirt'"));
    }

    SimpleMeshComponent->CreateMeshSection(0, Vertices, Indices, Material1, true, true); // Modifié pour prendre FVector
    SimpleMeshComponent ->SetMaterial(0, Material1);
    SimpleMeshComponent->CreateMeshSection(1, Vertices, Indices, Material2, true, true); // Modifié pour prendre FVector
    SimpleMeshComponent->SetMaterial(1, Material2);
    double EndTime = FPlatformTime::Seconds();
    double ElapsedTimeMs = (EndTime - StartTime) * 1000.0;
}

// SimpleMesh.cpp