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

#include "SubdivisablePlane.generated.h"


// Classe de l'acteur
UCLASS()
class ASubdivisablePlane : public AActor
{
    GENERATED_BODY()

public:
    ASubdivisablePlane();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SMC | Geometry Parameters")
        int32 NumDivisionsX = 16; // Valeur par défaut

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SMC | Geometry Parameters")
        int32 NumDivisionsY = 16; // Valeur par défaut

    void InitializePlaneGeometry();

public:

    virtual void Tick(float DeltaTime) override;
     
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "SMC | Mesh Parameters") //VisibleDefaultsOnly
    USceneComponent* Root;

    USimpleMeshComponent* SimpleMeshComponent;

    UPROPERTY(EditAnywhere, Category = "SMC | Material")
        UMaterialInterface* MaterialSlot;


};
