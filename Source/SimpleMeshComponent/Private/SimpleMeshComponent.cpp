
#include "SimpleMeshComponent.h"
#include "Materials/Material.h"
#include "SimpleMeshProxy.h"

#include "BodySetupEnums.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "MaterialDomain.h"

#include "Materials/MaterialRenderProxy.h"
#include "Engine/Engine.h"
#include "RenderUtils.h"
#include "SceneManagement.h"
#include "PhysicsEngine/BodySetup.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SceneInterface.h"
#include "StaticMeshResources.h"


#define LOCTEXT_NAMESPACE "FSimpleMeshComponentModule"

void FSimpleMeshComponentModule::StartupModule()
{

}

void FSimpleMeshComponentModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSimpleMeshComponentModule, SimpleMeshComponent)




USimpleMeshComponent::USimpleMeshComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


FPrimitiveSceneProxy* USimpleMeshComponent::CreateSceneProxy()
{
    return new FSimpleSceneProxy(this);
}

void USimpleMeshComponent::CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles, UMaterialInterface* Material, bool bSectionVisible)
{
    if (SectionIndex >= MeshSections.Num())
    {
        MeshSections.SetNum(SectionIndex + 1);
    }

    FSimpleSection& Section = MeshSections[SectionIndex];
    Section.VertexBuffer.Empty(Vertices.Num());
    Section.IndexBuffer = Triangles;

    //Try Material
    Section.MaterialIndex = GetMaterials().IndexOfByKey(Material);
    if (Section.MaterialIndex == INDEX_NONE)
    {
        Section.MaterialIndex = 0; // Fallback to default material if not found
    }


    Section.Visible = bSectionVisible;

    for (const FVector& Vertex : Vertices)
    {
        FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
        FDynamicMeshVertex DynamicVertex;
        DynamicVertex.Position = Vertex3f; // Utilisation de FVector3f
        Section.VertexBuffer.Add(DynamicVertex);
    }

    UpdateLocalBounds();
    MarkRenderStateDirty();
}

void USimpleMeshComponent::UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<uint32>& Triangles)
{
    if (MeshSections.IsValidIndex(SectionIndex))
    {
        FSimpleSection& Section = MeshSections[SectionIndex];
        Section.VertexBuffer.Empty(Vertices.Num());

        for (const FVector& Vertex : Vertices)
        {
            FVector3f Vertex3f(Vertex); // Conversion de FVector à FVector3f
            FDynamicMeshVertex DynamicVertex;
            DynamicVertex.Position = Vertex3f;
            Section.VertexBuffer.Add(DynamicVertex);
        }

        Section.IndexBuffer = Triangles;

        UpdateLocalBounds();
        MarkRenderStateDirty();
    }
}


void USimpleMeshComponent::RemoveMeshSection(int32 SectionIndex)
{
    if (MeshSections.IsValidIndex(SectionIndex))
    {
        MeshSections.RemoveAt(SectionIndex);
        UpdateLocalBounds();
        MarkRenderStateDirty();
    }
}


void USimpleMeshComponent::ClearAllMeshSections()
{
    MeshSections.Empty();

    UpdateLocalBounds();
    MarkRenderStateDirty();
}


FBoxSphereBounds USimpleMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    FBox BoundingBox(ForceInit);

    for (const FSimpleSection& Section : MeshSections)
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
    for (const FSimpleSection& Section : MeshSections)
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


int32 USimpleMeshComponent::GetNumSections() const
{
    return MeshSections.Num();
}


UMaterialInterface* USimpleMeshComponent::GetMaterial(int32 MaterialIndex) const
{
    if (MaterialIndex < GetMaterials().Num())
    {
        return GetMaterials()[MaterialIndex];
    }
    return Super::GetMaterial(0); // Fallback to default material
}
