// Copyright Epic Games, Inc. All Rights Reserved.
//---------EXAMPLE MESH---------//
// Licence: MIT License
// Created by: BenjaCorp at iolaCorpStudio
// Created at: 20/02/2024
//---------EXAMPLE MESH---------//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleMeshComponent.h"
#include "SimpleMesh.generated.h"




// Classe de l'acteur
UCLASS()
class ASimpleMesh : public AActor
{
    GENERATED_BODY()

public:
    ASimpleMesh();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SMC | Geometry Parameters")
        int32 NumDivisionsX = 256; // Valeur par d�faut

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SMC | Geometry Parameters")
        int32 NumDivisionsY = 256; // Valeur par d�faut

    void InitializePlaneGeometry();

public:
    virtual void Tick(float DeltaTime) override;
     
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "SMC | Mesh Parameters") //VisibleDefaultsOnly
    USceneComponent* Root;

    USimpleMeshComponent* SimpleMeshComponent;

    UPROPERTY(EditAnywhere, Category = "SMC | Material")
        UMaterialInterface* MaterialSlot;


};
