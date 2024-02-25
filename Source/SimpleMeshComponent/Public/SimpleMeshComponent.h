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

struct FKConvexElem;

/** One vertex for the Simple mesh, used for storing data internally */

USTRUCT(BlueprintType)
struct FSimpleMeshVertex
{
    GENERATED_BODY()
public:

    /** Vertex position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vertex)
        FVector Position;

    FSimpleMeshVertex()
        : Position(0.f, 0.f, 0.f)

    {}
};

/** One section of the procedural mesh. Each material has its own section. */

USTRUCT()
struct FSimpleMeshSection
{
    GENERATED_BODY()

public:

    /** Vertex buffer for this section */
        TArray<FDynamicMeshVertex> VertexBuffer;

    /** Index buffer for this section */
    UPROPERTY()
        TArray<uint32> IndexBuffer;
    /** Local bounding box of section */

    UPROPERTY()
    int32 MaterialIndex;

    UPROPERTY()
        FBox SectionLocalBox;

    /** Should we build collision data for triangles in this section */
    UPROPERTY()
        bool bEnableCollision;

    /** Should we display this section */
    UPROPERTY()
        bool Visible;

    FSimpleMeshSection()
        : SectionLocalBox(ForceInit)
        , bEnableCollision(false)
        , Visible(true)
    {}

    /** Reset this section, clear all mesh info. */
    void Reset()
    {
        VertexBuffer.Empty();
        IndexBuffer.Empty();
        SectionLocalBox.Init();
        bEnableCollision = false;
        Visible = true;
    }
};





UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class SIMPLEMESHCOMPONENT_API USimpleMeshComponent  : public UMeshComponent, public IInterface_CollisionDataProvider
{
    GENERATED_BODY()

public:

// Constructeur
    USimpleMeshComponent(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Create Mesh Section", AutoCreateRefTerm = "Vertices, Triangles , Material, Visibility, Collision"))
        void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, UMaterialInterface* Material, bool bSectionVisible = true, bool bCreateCollision = false);
    /**
     *	Create/replace a section for this procedural mesh component.
     *	@param	SectionIndex		Index of the section to create or replace.
     *	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
     *	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
     *	@param	Material			
     *	@param	bVisibility     	
     */

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Update Mesh Section", AutoCreateRefTerm = "Vertices, Triangles, Collision"))
    void UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, bool bCreateCollision);
    /**
     *	Create/replace a section for this procedural mesh component.
     *	@param	SectionIndex		Index of the section to create or replace.
     *	@param	Vertices			Vertex buffer of all vertex positions to use for this mesh section.
     *	@param	Triangles			Index buffer indicating which vertices make up each triangle. Length must be a multiple of 3.
     */

    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh", meta = (DisplayName = "Remove Mesh Section", AutoCreateRefTerm = ""))
    void RemoveMeshSection(int32 SectionIndex);

    /** Clear all mesh sections */
    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh")
        void ClearAllMeshSections();

    /** Returns number of sections currently created for this component */
    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh")
    int32 GetNumSections() const;

    /** Add simple collision convex to this component */
    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh")
    void AddCollisionConvexMesh(TArray<FVector> ConvexVerts);

    /** Remove collision meshes from this component */
    UFUNCTION(BlueprintCallable, Category = "Components|SimpleMesh")
    void ClearCollisionConvexMeshes();

    /** Function to replace _all_ simple collision in one go */
    void SetCollisionConvexMeshes(const TArray< TArray<FVector> >& ConvexMeshes);

    //~ Begin Interface_CollisionDataProvider Interface
    virtual bool GetTriMeshSizeEstimates(struct FTriMeshCollisionDataEstimates& OutTriMeshEstimates, bool bInUseAllTriData) const override;
    virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
    virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
    virtual bool WantsNegXTriMesh() override { return false; }
    //~ End Interface_CollisionDataProvider Interface


    virtual void PostLoad() override;


    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual class UBodySetup* GetBodySetup() override;

    virtual UMaterialInterface* GetMaterial(int32 MaterialIndex) const override;

    TArray<FSimpleMeshSection> MeshSections; // Stocke les sections de maillage
    
    //Collision

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision|SimpleMesh")
        bool bUseComplexAsSimpleCollision;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision|SimpleMesh")
        bool bUseAsyncCooking;

    /** Collision data */
    UPROPERTY(Instanced)
        TObjectPtr<class UBodySetup> SimpleMeshBodySetup;

    /** Replace a section with new section geometry */
    void SetSimpleMeshSection(int32 SectionIndex, const FSimpleMeshSection& Section);

    //~ Begin USceneComponent Interface.
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    //~ Begin USceneComponent Interface.


protected:
	
    void UpdateLocalBounds(); // Mise à jour des limites locales basées sur les sections de maillage


private:
   
    FBoxSphereBounds LocalBounds; // Limites locales du maillage

    /** Ensure ProcMeshBodySetup is allocated and configured */
    void CreateSimpleMeshBodySetup();

    /** Helper to create new body setup objects */
    UBodySetup* CreateBodySetupHelper();

    /** Mark collision data as dirty, and re-create on instance if necessary */
    void UpdateCollision();

    void FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup);

    /** Convex shapes used for simple collision */
    UPROPERTY()
        TArray<FKConvexElem> CollisionConvexElems;

    /** Queue for async body setups that are being cooked */
    UPROPERTY(transient)
        TArray<TObjectPtr<UBodySetup>> AsyncBodySetupQueue;

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
            FSimpleMeshSection& MeshSection = Component->MeshSections[i];
            Options.bIsVisible = MeshSection.Visible;
            Sections[i] = new FSimpleMeshSceneSection(MeshSection.VertexBuffer, MeshSection.IndexBuffer,
                Component->GetMaterial(MeshSection.MaterialIndex), Options, FL);
        }

        SectionsUpdated();
    }
};



// Module Plugin

class FSimpleMeshComponentModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};