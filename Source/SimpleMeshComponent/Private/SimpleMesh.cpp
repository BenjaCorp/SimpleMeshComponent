

#include "SimpleMesh.h"
#include "SimpleMeshComponent.h"
#include "DynamicMeshBuilder.h"


ASimpleMesh::ASimpleMesh()
{
    PrimaryActorTick.bCanEverTick = true;
    

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
    Root->SetMobility(EComponentMobility::Static);
    SimpleMeshComponent = CreateDefaultSubobject<USimpleMeshComponent>(TEXT("SimpleMeshComponent"));
    SimpleMeshComponent->SetupAttachment(Root);
    SimpleMeshComponent->SetMobility(EComponentMobility::Static);
}

void ASimpleMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    InitializePlaneGeometry();
}

void ASimpleMesh::BeginPlay()
{
    Super::BeginPlay();
    InitializePlaneGeometry();
}

void ASimpleMesh::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASimpleMesh::InitializePlaneGeometry()
{
    double StartTime = FPlatformTime::Seconds();
    if (NumDivisionsX < 1 || NumDivisionsY < 1) return;

    TArray<FVector> Vertices; // Modification pour utiliser FVector directement
    TArray<uint32> Indices;

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
    UE_LOG(LogTemp, Warning, TEXT("[SMC] Not Find"));
    UMaterialInterface* Material = MaterialSlot;

    if (!Material) {
        Material = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial'"));
        UE_LOG(LogTemp, Warning, TEXT("[SMC] Reset DefaultMaterial"));
    }

    SimpleMeshComponent->CreateMeshSection(0, Vertices, Indices, Material, true); // Modifié pour prendre FVector
    SimpleMeshComponent ->SetMaterial(0, Material);

    double EndTime = FPlatformTime::Seconds();
    double ElapsedTimeMs = (EndTime - StartTime) * 1000.0;
    UE_LOG(LogTemp, Warning, TEXT("[SMC] Geometry was generated and drawn -> %f ms."), ElapsedTimeMs);
}


// SimpleMesh.cpp









 



