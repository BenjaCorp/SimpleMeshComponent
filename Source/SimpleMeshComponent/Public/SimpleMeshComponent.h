// SimpleMeshComponent.h
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

    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

    void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles, UMaterialInterface* Material, bool bSectionVisible = true);
    void UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles);
    void RemoveMeshSection(int32 SectionIndex);
    void ClearAllMeshSections();

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

    UBodySetup* BodySetup;
};



// Module Plugin

class FSimpleMeshComponentModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};