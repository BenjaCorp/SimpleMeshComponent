// Copyright Epic Games, Inc. All Rights Reserved.

//---------SIMPLEMESHCOMPONENT.CPP---------//
// Licence: MIT License                    //
// Created by: BenjaCorp at iolaCorpStudio //
// Created at: 20/02/2024                  //
// Copyright 2024 BenjaCorp                //
//---------SIMPLEMESHCOMPONENT.CPP---------//


#include "SimpleMeshComponent.h"
#include "SimpleMeshProxy.h"
#include "BodySetupEnums.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "Engine/Engine.h"
#include "RenderUtils.h"
#include "SceneManagement.h"
#include "PhysicsEngine/BodySetup.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SceneInterface.h"
#include "StaticMeshResources.h"
#include "RayTracingInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SimpleMeshComponent)
DEFINE_LOG_CATEGORY_STATIC(LogSimpleComponent, Log, All);

#define LOCTEXT_NAMESPACE "FSimpleMeshComponentModule"

void FSimpleMeshComponentModule::StartupModule() {}
void FSimpleMeshComponentModule::ShutdownModule() {}


/** Resource array to pass  */
class FSimpleMeshVertexResourceArray : public FResourceArrayInterface
{
public:
    FSimpleMeshVertexResourceArray(void* InData, uint32 InSize)
        : Data(InData)
        , Size(InSize) {}

    virtual const void* GetResourceData() const override { return Data; }
    virtual uint32 GetResourceDataSize() const override { return Size; }
    virtual void Discard() override { }
    virtual bool IsStatic() const override { return false; }
    virtual bool GetAllowCPUAccess() const override { return false; }
    virtual void SetAllowCPUAccess(bool bInNeedsCPUAccess) override { }

private:
    void* Data;
    uint32 Size;
};


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSimpleMeshComponentModule, SimpleMeshComponent)


USimpleMeshComponent::USimpleMeshComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUseComplexAsSimpleCollision = true;
}

void USimpleMeshComponent::PostLoad()
{
    Super::PostLoad();

    if (SimpleMeshBodySetup && IsTemplate())
    {
        SimpleMeshBodySetup->SetFlags(RF_Public | RF_ArchetypeObject);
    }
}


FPrimitiveSceneProxy* USimpleMeshComponent::CreateSceneProxy()
{
    //SCOPE_CYCLE_COUNTER(STAT_SimpleMesh_CreateSceneProxy);
    return new FSimpleSceneProxy(this);
}

static void ConvertSimpleMeshToDynMeshVertex(FDynamicMeshVertex& Vert, const FSimpleMeshVertex& SimpleVert)
{
    Vert.Position = (FVector3f)SimpleVert.Position;
}


void USimpleMeshComponent::CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, UMaterialInterface* Material, bool bSectionVisible, bool bCreateCollision)
{
    // Vérifier que les tableaux de vertices et de triangles ne sont pas vides avant de continuer.
    if (Vertices.IsEmpty() || Triangles.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateMeshSection called with empty vertices or triangles array. Skipping section creation."));
        return; // Sortie anticipée pour éviter un crash dû à des données vides.
    }

    // Ajuster la taille du tableau MeshSections pour s'assurer qu'il peut contenir la nouvelle section.
    if (SectionIndex >= MeshSections.Num())
    {
        MeshSections.SetNum(SectionIndex + 1, true); // Utilisez true pour initialiser les nouvelles sections.
    }

    FSimpleMeshSection& Section = MeshSections[SectionIndex];
    Section.Reset(); // Réinitialiser la section pour nettoyer les données précédentes.

   // Section.MaterialIndex = GetMaterials().IndexOfByKey(Material);
    if (Section.MaterialIndex == INDEX_NONE)
    {
        Section.MaterialIndex = 0; // Fallback to default material if not found
    }

    // Note: Cette approche suppose que Material n'est pas null. Ajoutez des vérifications si Material peut être null.
    Section.MaterialIndex = Materials.AddUnique(Material); // Assurez-vous que Materials est accessible et correctement défini.
   
    Section.Visible = bSectionVisible;

    // Conversion de FVector à FVector3f et ajout au VertexBuffer
    for (const FVector& Vertex : Vertices)
    {
        FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
        FDynamicMeshVertex DynamicVertex;
        DynamicVertex.Position = Vertex3f;
        Section.VertexBuffer.Add(DynamicVertex);
    }

    // Conversion de int32 à uint32 pour les indices des triangles
    for (const int32& Index : Triangles)
    {
        Section.IndexBuffer.Add(static_cast<uint32>(Index));
    }

    // Activer la collision pour cette section, si demandé
    Section.bEnableCollision = bCreateCollision;



    // Mise à jour des limites locales, de la collision et du marquage pour la recréation de l'état de rendu
    UpdateLocalBounds();
    UpdateCollision();
    MarkRenderStateDirty();
}


// Maybe Collision make Crash at Editor

void USimpleMeshComponent::UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, bool bCreateCollision)
{
    if (MeshSections.IsValidIndex(SectionIndex))
    {

        FSimpleMeshSection& Section = MeshSections[SectionIndex];
        Section.VertexBuffer.Empty(Vertices.Num());

        // Conversion de FVector à FVector3f et ajout au VertexBuffer
        for (const FVector& Vertex : Vertices)
        {
            FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
            FDynamicMeshVertex DynamicVertex;
            DynamicVertex.Position = Vertex3f;
            Section.VertexBuffer.Add(DynamicVertex);
        }

        // Conversion de int32 à uint32 pour les indices des triangles
        Section.IndexBuffer.Empty(Triangles.Num());
        for (const int32& Index : Triangles)
        {
            Section.IndexBuffer.Add(static_cast<uint32>(Index));
        }

        // If we have collision enabled on this section, update that too
        if (Section.bEnableCollision)
        {
            TArray<FVector> CollisionPositions;

            // We have one collision mesh for all sections, so need to build array of _all_ positions
            for (const FSimpleMeshSection& CollisionSection : MeshSections)
            {
                // If section has collision, copy it
                if (CollisionSection.bEnableCollision)
                {
                    for (int32 VertIdx = 0; VertIdx < CollisionSection.VertexBuffer.Num(); VertIdx++)
                    {
                       // CollisionPositions.Add(CollisionSection.VertexBuffer[VertIdx].Position);
                    }
                }
            }

            // Pass new positions to trimesh
            BodyInstance.UpdateTriMeshVertices(CollisionPositions);
        }

        UpdateLocalBounds();
        UpdateCollision(); //Good for Update Collision
        MarkRenderStateDirty();
    }
}

void USimpleMeshComponent::RemoveMeshSection(int32 SectionIndex)
{
    if (MeshSections.IsValidIndex(SectionIndex))
    {
        MeshSections.RemoveAt(SectionIndex);
        UpdateLocalBounds();
        UpdateCollision();
        MarkRenderStateDirty();
    }
}

void USimpleMeshComponent::ClearAllMeshSections()
{
    MeshSections.Empty();
    UpdateLocalBounds();
    UpdateCollision();
    MarkRenderStateDirty();
}


bool USimpleMeshComponent::GetTriMeshSizeEstimates(struct FTriMeshCollisionDataEstimates& OutTriMeshEstimates, bool bInUseAllTriData) const
{
    for (const FSimpleMeshSection& Section : MeshSections)
    {
        if (Section.bEnableCollision)
        {
            OutTriMeshEstimates.VerticeCount += Section.VertexBuffer.Num();
        }
    }

    return true;
}

FBoxSphereBounds USimpleMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    FBox BoundingBox(ForceInit);

    for (const FSimpleMeshSection& Section : MeshSections)
    {
        for (const FDynamicMeshVertex& Vertex : Section.VertexBuffer)
        {
            // Convertir explicitement FVector3f en FVector avant de l'ajouter à BoundingBox
            FVector VertexPosition = FVector(Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
            BoundingBox += VertexPosition;
        }
    }

    return FBoxSphereBounds(BoundingBox).TransformBy(LocalToWorld);
}

void USimpleMeshComponent::UpdateLocalBounds()
{
    FBox LocalBox(ForceInit);
    for (const FSimpleMeshSection& Section : MeshSections)
    {
        for (const FDynamicMeshVertex& Vertex : Section.VertexBuffer)
        {
            // Convertir explicitement FVector3f en FVector avant de l'ajouter à LocalBox
            FVector VertexPosition = FVector(Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
            LocalBox += VertexPosition;
        }
    }
    LocalBounds = FBoxSphereBounds(LocalBox);
    UpdateBounds();
}

bool USimpleMeshComponent::DoesSectionExist(int32 SectionIndex) const
{
    return MeshSections.IsValidIndex(SectionIndex) ;
}

int32 USimpleMeshComponent::GetNumSections() const
{
    return MeshSections.Num();
}

void USimpleMeshComponent::AddCollisionConvexMesh(TArray<FVector> ConvexVerts)
{
    if (ConvexVerts.Num() >= 4)
    {
        // New element
        FKConvexElem NewConvexElem;
        // Copy in vertex info
        NewConvexElem.VertexData = ConvexVerts;
        // Update bounding box
        NewConvexElem.ElemBox = FBox(ConvexVerts);
        // Add to array of convex elements
        CollisionConvexElems.Add(NewConvexElem);
        // Refresh collision
        UpdateCollision();
    }
}



void USimpleMeshComponent::UpdateCollision()
{
   // SCOPE_CYCLE_COUNTER(STAT_SimpleMesh_UpdateCollision);
    
    UWorld* World = GetWorld();
    const bool bUseAsyncCook = World && World->IsGameWorld() && bUseAsyncCooking;
    if (bUseAsyncCook)
    {
        
        // Abort all previous ones still standing
        for (UBodySetup* OldBody : AsyncBodySetupQueue)
        {
            OldBody->AbortPhysicsMeshAsyncCreation();
        }

        AsyncBodySetupQueue.Add(CreateBodySetupHelper());
    }
    else
    {
        AsyncBodySetupQueue.Empty();	//If for some reason we modified the async at runtime, just clear any pending async body setups
        CreateSimpleMeshBodySetup(); 
        
    }

    UBodySetup* UseBodySetup = bUseAsyncCook ? AsyncBodySetupQueue.Last() : SimpleMeshBodySetup;

    // Fill in simple collision convex elements
    UseBodySetup->AggGeom.ConvexElems = CollisionConvexElems;

    // Set trace flag
    UseBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

    if (bUseAsyncCook)
    {
        
        UseBodySetup->CreatePhysicsMeshesAsync(FOnAsyncPhysicsCookFinished::CreateUObject(this, &USimpleMeshComponent::FinishPhysicsAsyncCook, UseBodySetup));
    }
    else
    {
        // New GUID as collision has changed
        UseBodySetup->BodySetupGuid = FGuid::NewGuid();
        // Also we want cooked data for this
        UseBodySetup->bHasCookedCollisionData = true;
        UseBodySetup->InvalidatePhysicsData();
        UseBodySetup->CreatePhysicsMeshes();
        RecreatePhysicsState();
    }
}


UBodySetup* USimpleMeshComponent::CreateBodySetupHelper()
{
    // The body setup in a template needs to be public since the property is Tnstanced and thus is the archetype of the instance meaning there is a direct reference
    UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public | RF_ArchetypeObject : RF_NoFlags));
    NewBodySetup->BodySetupGuid = FGuid::NewGuid();

    NewBodySetup->bGenerateMirroredCollision = false;
    NewBodySetup->bDoubleSidedGeometry = true;
    NewBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

    return NewBodySetup;
}


void USimpleMeshComponent::CreateSimpleMeshBodySetup()
{
    if (SimpleMeshBodySetup == nullptr)
    {
        SimpleMeshBodySetup = CreateBodySetupHelper();
    }
}

void USimpleMeshComponent::FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup)
{
    TArray<UBodySetup*> NewQueue;
    NewQueue.Reserve(AsyncBodySetupQueue.Num());

    int32 FoundIdx;
    if (AsyncBodySetupQueue.Find(FinishedBodySetup, FoundIdx))
    {
        if (bSuccess)
        {
            //The new body was found in the array meaning it's newer so use it
            SimpleMeshBodySetup = FinishedBodySetup;
            RecreatePhysicsState();

            //remove any async body setups that were requested before this one
            for (int32 AsyncIdx = FoundIdx + 1; AsyncIdx < AsyncBodySetupQueue.Num(); ++AsyncIdx)
            {
                NewQueue.Add(AsyncBodySetupQueue[AsyncIdx]);
            }

            AsyncBodySetupQueue = NewQueue;
        }
        else
        {
            AsyncBodySetupQueue.RemoveAt(FoundIdx);
        }
    }
}

void USimpleMeshComponent::SetCollisionConvexMeshes(const TArray< TArray<FVector> >& ConvexMeshes)
{
    CollisionConvexElems.Reset();

    // Create element for each convex mesh
    for (int32 ConvexIndex = 0; ConvexIndex < ConvexMeshes.Num(); ConvexIndex++)
    {
        FKConvexElem NewConvexElem;
        NewConvexElem.VertexData = ConvexMeshes[ConvexIndex];
        NewConvexElem.ElemBox = FBox(NewConvexElem.VertexData);

        CollisionConvexElems.Add(NewConvexElem);
    }

    UpdateCollision();
}


void USimpleMeshComponent::ClearCollisionConvexMeshes()
{
    // Empty simple collision info
    CollisionConvexElems.Empty();
    // Refresh collision
    UpdateCollision();
}



bool USimpleMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
    int32 VertexBase = 0; // Base vertex index for current section

    // For each section..
    for (int32 SectionCnt = 0; SectionCnt < MeshSections.Num(); SectionCnt++)
    {
        FSimpleMeshSection& Section = MeshSections[SectionCnt];
        // Do we have collision enabled?
        if (Section.bEnableCollision)
        {


            // Copy vert data
            for (int32 VertIdx = 0; VertIdx < Section.VertexBuffer.Num(); VertIdx++)
            {
                CollisionData->Vertices.Add((FVector3f)Section.VertexBuffer[VertIdx].Position);

            }

            // Copy triangle data
            const int32 NumTriangles = Section.IndexBuffer.Num() / 3;
            for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
            {
                // Need to add base offset for indices
                FTriIndices Triangle;
                Triangle.v0 = Section.IndexBuffer[(TriIdx * 3) + 0] + VertexBase;
                Triangle.v1 = Section.IndexBuffer[(TriIdx * 3) + 1] + VertexBase;
                Triangle.v2 = Section.IndexBuffer[(TriIdx * 3) + 2] + VertexBase;
                CollisionData->Indices.Add(Triangle);

                // Also store material info
                CollisionData->MaterialIndices.Add(SectionCnt);

            }

            // Remember the base index that new verts will be added from in next section
            VertexBase = CollisionData->Vertices.Num();
        }
    }

    CollisionData->bFlipNormals = true;
    CollisionData->bDeformableMesh = true;
    CollisionData->bFastCook = true;
    return true;
}

bool USimpleMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
    for (const FSimpleMeshSection& Section : MeshSections)
    {
        if (Section.IndexBuffer.Num() >= 3 && Section.bEnableCollision)
        {
            return true;
        }
    }

    return false;
}


void USimpleMeshComponent::SetSimpleMeshSection(int32 SectionIndex, const FSimpleMeshSection& Section)
{
    // Ensure sections array is long enough
    if (SectionIndex >= MeshSections.Num())
    {
        MeshSections.SetNum(SectionIndex + 1, false);
    }

    MeshSections[SectionIndex] = Section;

    UpdateLocalBounds(); // Update overall bounds
    UpdateCollision(); // Mark collision as dirty
    MarkRenderStateDirty(); // New section requires recreating scene proxy
}


FSimpleMeshSection* USimpleMeshComponent::GetSimpleMeshSection(int32 SectionIndex)
{
    if (SectionIndex < MeshSections.Num())
    {
        return &MeshSections[SectionIndex];
    }
    else
    {
        return nullptr;
    }
}


UBodySetup* USimpleMeshComponent::GetBodySetup()
{
    CreateSimpleMeshBodySetup();
    return SimpleMeshBodySetup;
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//     Fonction Create/UpdateMeshSection Withnot Uint32 Convertion
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//


//void USimpleMeshComponent::CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles, UMaterialInterface* Material, bool bSectionVisible)
//{
//    if (SectionIndex >= MeshSections.Num())
//    {
//        MeshSections.SetNum(SectionIndex + 1);
//    }
//
//    FSimpleSection& Section = MeshSections[SectionIndex];
//    Section.VertexBuffer.Empty(Vertices.Num());
//    Section.IndexBuffer = Triangles;
//
//    //Try Material
//    Section.MaterialIndex = GetMaterials().IndexOfByKey(Material);
//    if (Section.MaterialIndex == INDEX_NONE)
//    {
//        Section.MaterialIndex = 0; // Fallback to default material if not found
//    }
//
//
//    Section.Visible = bSectionVisible;
//
//    for (const FVector& Vertex : Vertices)
//    {
//        FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
//        FDynamicMeshVertex DynamicVertex;
//        DynamicVertex.Position = Vertex3f; // Utilisation de FVector3f
//        Section.VertexBuffer.Add(DynamicVertex);
//    }
//
//    UpdateLocalBounds();
//    MarkRenderStateDirty();
//}

//void USimpleMeshComponent::UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles)
//{
//    if (MeshSections.IsValidIndex(SectionIndex))
//    {
//        FSimpleSection& Section = MeshSections[SectionIndex];
//        Section.VertexBuffer.Empty(Vertices.Num());
//
//        for (const FVector& Vertex : Vertices)
//        {
//            FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
//            FDynamicMeshVertex DynamicVertex;
//            DynamicVertex.Position = Vertex3f;
//            Section.VertexBuffer.Add(DynamicVertex);
//        }
//
//        Section.IndexBuffer = Triangles;
//
//        UpdateLocalBounds();
//        MarkRenderStateDirty();
//    }
//}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
