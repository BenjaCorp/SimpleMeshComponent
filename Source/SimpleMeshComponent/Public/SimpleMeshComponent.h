// Copyright Epic Games, Inc. All Rights Reserved.
//---------SIMPLEMESHCOMPONENT.H---------//
// Licence: MIT License
// Created by: BenjaCorp at iolaCorpStudio
// Created at: 20/02/2024
//---------SIMPLEMESHCOMPONENT.H---------//
#pragma once

#include "CoreMinimal.h"
#include "SimpleMeshProxy.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Components/MeshComponent.h"
#include "Modules/ModuleManager.h"

#include "SimpleMeshComponent.generated.h"

// Structure pour les sections de maillage
USTRUCT()
struct FSimpleSection
{
    GENERATED_BODY()

    TArray<FDynamicMeshVertex> VertexBuffer;
    TArray<uint32> IndexBuffer;
    int32 MaterialIndex;
    bool Visible;

    FSimpleSection() : MaterialIndex(0), Visible(true) {}
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIMPLEMESHCOMPONENT_API USimpleMeshComponent  : public UMeshComponent
{
    GENERATED_BODY()

public:

// Constructeur
    
    USimpleMeshComponent();

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Create Mesh Section", AutoCreateRefTerm = "Vertices, Triangles , Material, Visibility"))
        void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, UMaterialInterface* Material, bool bSectionVisible = true);

//    void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, UMaterialInterface* Material, bool bSectionVisible = true);
    /**
     *	Create/replace a section for this procedural mesh component.
     *	@param	SectionIndex		Index of the section to create or replace.
     *	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
     *	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
     *	@param	Material			
     *	@param	bVisibility     	
     */

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Update Mesh Section", AutoCreateRefTerm = "Vertices, Triangles"))
    void UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles);

    /**
     *	Create/replace a section for this procedural mesh component.
     *	@param	SectionIndex		Index of the section to create or replace.
     *	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
     *	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
     */

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Remove Mesh Section", AutoCreateRefTerm = ""))
    void RemoveMeshSection(int32 SectionIndex);


    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Clear All Mesh Sections", AutoCreateRefTerm = ""))
    void ClearAllMeshSections();

    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

    virtual UMaterialInterface* GetMaterial(int32 MaterialIndex) const override;

    TArray<FSimpleSection> MeshSections; // Stocke les sections de maillage

protected:
	
    void UpdateLocalBounds(); // Mise à jour des limites locales basées sur les sections de maillage


private:
   
    FBoxSphereBounds LocalBounds; // Limites locales du maillage

    int32 GetNumSections() const; // Renvoie le nombre de sections de maillage

};

// Scene Proxy to RenderThread

class FSimpleSceneProxy : public FSimpleMeshSceneProxy
{
public:
  
    FSimpleSceneProxy(USimpleMeshComponent* Component) :
        FSimpleMeshSceneProxy(Component)
    {
        int32 SectionCnt = Component->MeshSections.Num();
        Sections.AddZeroed(SectionCnt);
        ERHIFeatureLevel::Type FL = GetScene().GetFeatureLevel();
        bShouldRenderStatic = !IsMovable();
        FSimpleMeshSectionOptions Options;
        Options.bCastsShadow = true;
        Options.bIsMainPassRenderable = Component->bRenderInMainPass;
        Options.bShouldRenderStatic = bShouldRenderStatic;

        for (int i = 0; i < SectionCnt; i++)
        {
            FSimpleSection& MeshSection = Component->MeshSections[i];
            Options.bIsVisible = MeshSection.Visible;
            Sections[i] = new FSimpleMeshSceneSection(MeshSection.VertexBuffer, MeshSection.IndexBuffer,
                Component->GetMaterial(MeshSection.MaterialIndex), Options, FL);
        }

        SectionsUpdated();
    }

private:

    //UBodySetup* BodySetup; // Todo Colision
};



// Module Plugin

class FSimpleMeshComponentModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};