/*
SimpleMeshProxy.h - https://github.com/StevenChristy/SimpleMeshProxy

MIT License

Copyright (c) 2016-2020 TriAxis Games L.L.C.
Copyright 2022 Steven Christy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include "RenderResource.h"
#include "RenderingThread.h"
#include "PrimitiveSceneProxy.h"
#include "MaterialShared.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "PhysicsEngine/BodySetup.h"
#include "DynamicMeshBuilder.h"
#include "StaticMeshResources.h"
#include "RayTracingInstance.h"
#if ENGINE_MAJOR_VERSION==4
#include "TessellationRendering.h"
#endif

DECLARE_STATS_GROUP(TEXT("SimpleMeshProxy"), STATGROUP_SimpleMeshProxy, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("SimpleMeshProxy - CreateMeshBatch"), STAT_SimpleMeshSceneProxy_CreateMeshBatch, STATGROUP_SimpleMeshProxy);
DECLARE_CYCLE_STAT(TEXT("SimpleMeshProxy - DrawStaticMeshElements"), STAT_SimpleMeshSceneProxy_DrawStaticMeshElements, STATGROUP_SimpleMeshProxy);
DECLARE_CYCLE_STAT(TEXT("SimpleMeshProxy - GetDynamicMeshElements"), STAT_SimpleMeshSceneProxy_GetDynamicMeshElements, STATGROUP_SimpleMeshProxy);
DECLARE_CYCLE_STAT(TEXT("SimpleMeshProxy - GetDynamicRayTracingInstances"), STAT_SimpleMeshSceneProxy_GetDynamicRayTracingInstances, STATGROUP_SimpleMeshProxy);


struct FSimpleMeshSectionOptions
{
	uint32 bIsValid : 1;
	uint32 bIsVisible : 1;
	uint32 bIsMainPassRenderable : 1;
	uint32 bCastsShadow : 1;
	uint32 bShouldRenderStatic : 1;

	FSimpleMeshSectionOptions() : bIsValid(false), bIsVisible(false), bIsMainPassRenderable(false),
		bCastsShadow(false), bShouldRenderStatic(false)
	{
		
	}
};

class FSimpleMeshSceneSection
{
public:
	int NumPrimitives = 0;
	int MaxVertex = 0;
	int LODIndex = 0;
	UMaterialInterface* Material = nullptr;
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;
	FSimpleMeshSectionOptions Options;
#if RHI_RAYTRACING
	FRayTracingGeometry RayTracingGeometry;
#endif

	FSimpleMeshSceneSection(TArray<FDynamicMeshVertex>& InVertexBuffer, TArray<uint32>& InIndexBuffer,
	                                 UMaterialInterface* InMaterial, FSimpleMeshSectionOptions InOptions,
	                                 ERHIFeatureLevel::Type InFeatureLevel, int InLODIndex = 0, uint8 MaxTexcoords = MAX_TEXCOORDS)
		: VertexFactory(InFeatureLevel, "FSimpleMeshSceneSection")
	{
		Options = InOptions;	
		
		IndexBuffer.Indices = InIndexBuffer;
		VertexBuffers.InitFromDynamicVertex(&VertexFactory, InVertexBuffer, MaxTexcoords);
		NumPrimitives = IndexBuffer.Indices.Num() / 3;
		MaxVertex = VertexBuffers.PositionVertexBuffer.GetNumVertices()-1;
		LODIndex = InLODIndex;

		CheckValidity();

		BeginInitResource(&VertexBuffers.PositionVertexBuffer);
		BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
		BeginInitResource(&VertexBuffers.ColorVertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		if (InMaterial == nullptr)
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		else
			Material = InMaterial;

#if RHI_RAYTRACING
		/*if (IsRayTracingEnabled())
		{
			ENQUEUE_RENDER_COMMAND(InitProceduralMeshRayTracingGeometry)([this](FRHICommandListImmediate& RHICmdList)
			{

				FRayTracingGeometryInitializer Initializer;
				Initializer.DebugName = FName("FSimpleMeshSceneSection");
				Initializer.IndexBuffer = nullptr;
				Initializer.TotalPrimitiveCount = 0;
				Initializer.GeometryType = RTGT_Triangles;
				Initializer.bFastBuild = true;
				Initializer.bAllowUpdate = false;

				RayTracingGeometry.SetInitializer(Initializer);
				RayTracingGeometry.InitResource();

				RayTracingGeometry.Initializer.IndexBuffer = IndexBuffer.IndexBufferRHI;
				RayTracingGeometry.Initializer.TotalPrimitiveCount = IndexBuffer.Indices.Num() / 3;

				FRayTracingGeometrySegment Segment;
				Segment.VertexBuffer = VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
				Segment.NumPrimitives = RayTracingGeometry.Initializer.TotalPrimitiveCount;
				RayTracingGeometry.Initializer.Segments.Add(Segment);

				RayTracingGeometry.UpdateRHI();
			});
		}*/
#endif
	}

	~FSimpleMeshSceneSection()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();

#if RHI_RAYTRACING
		if (IsRayTracingEnabled())
		{
			RayTracingGeometry.ReleaseResource();
		}
#endif
	}
	
	FORCEINLINE bool CanRender() const 
	{
		return Options.bIsValid;
		UE_LOG(LogTemp, Warning, TEXT("RenderPass = ok?"));
	}

	FORCEINLINE bool ShouldRenderMainPass() const { return Options.bIsMainPassRenderable; }
	FORCEINLINE bool IsStaticSection() const { return Options.bShouldRenderStatic; }

	FORCEINLINE bool ShouldRender() const { return CanRender() && Options.bIsVisible; }
	FORCEINLINE bool ShouldRenderStaticPath() const { return ShouldRender() && ShouldRenderMainPass() && IsStaticSection(); }
	FORCEINLINE bool ShouldRenderDynamicPath() const { return ShouldRender() && ShouldRenderMainPass() && !IsStaticSection(); }
	FORCEINLINE bool ShouldRenderShadow() const { return ShouldRender() && Options.bCastsShadow; }

	FORCEINLINE bool ShouldRenderDynamicPathRayTracing() const { return ShouldRender(); }


	void CheckValidity()
	{
		Options.bIsValid = true;
		Options.bIsValid &= VertexBuffers.PositionVertexBuffer.GetNumVertices() >= 3;
		Options.bIsValid &= IndexBuffer.Indices.Num() >= 3 && (IndexBuffer.Indices.Num() % 3)==0;
	}
};

class FSimpleMeshSceneProxy: public FPrimitiveSceneProxy
{
protected:
	FMaterialRelevance MaterialRelevance;
	UBodySetup* BodySetup;
	TArray<FSimpleMeshSceneSection*> Sections;
	uint32 bShouldRenderStatic : 1;
	uint32 bAnyMaterialUsesDithering : 1;

public:
	size_t GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FSimpleMeshSceneProxy(UPrimitiveComponent* Component) : FPrimitiveSceneProxy(Component), BodySetup(Component->GetBodySetup())
	{
		bAnyMaterialUsesDithering = false;
		bCastDynamicShadow = true;
		bShouldRenderStatic = true;
		const auto FeatureLevel = GetScene().GetFeatureLevel();
		bVFRequiresPrimitiveUniformBuffer = !UseGPUScene(GMaxRHIShaderPlatform, FeatureLevel);
		bStaticElementsAlwaysUseProxyPrimitiveUniformBuffer = true;
		bVerifyUsedMaterials = false;
	}

	~FSimpleMeshSceneProxy()
	{
		for (FSimpleMeshSceneSection* Section : Sections)
		{
			if (Section != nullptr)
			{
				delete Section;
			}
		}
	}

	void SectionsUpdated()
	{
		//UE_LOG(LogTemp, Warning, TEXT("[SMC] Start Section Update"));
		const auto FeatureLevel = GetScene().GetFeatureLevel();
		for (int x = 0, xc = Sections.Num(); x < xc; x++)
		{
			check(Sections[x]!=nullptr);
			auto &Section = *(Sections[x]);
			check(Section.Material);
			check(Section.CanRender());
			MaterialRelevance |= Section.Material->GetRelevance(FeatureLevel);
			bAnyMaterialUsesDithering |= Section.Material->IsDitheredLODTransition();
			//UE_LOG(LogTemp, Warning, TEXT("[SMC] SectionsUpdated-on proxy"));
		}
	}

	void CreateMeshBatch(FMeshBatch& MeshBatch, const FSimpleMeshSceneSection& Section, int32 SectionId, FMaterialRenderProxy* WireframeMaterial, bool bForRayTracing) const
	{
		SCOPE_CYCLE_COUNTER(STAT_SimpleMeshSceneProxy_CreateMeshBatch);
		
		// Should we be rendering in wireframe?
		const bool bRenderWireframe = WireframeMaterial != nullptr;

#if ENGINE_MAJOR_VERSION==4
		// Decide if we should be using adjacency information for this material
		const bool bWantsAdjacencyInfo = !bForRayTracing && !bRenderWireframe && RequiresAdjacencyInformation(Section.Material, Section.VertexFactory.GetType(), GetScene().GetFeatureLevel());
		check(!bWantsAdjacencyInfo);
#endif

		const FMaterialRenderProxy* MaterialRenderProxy = Section.Material->GetRenderProxy();

		check(Section.VertexFactory.IsInitialized());
		MeshBatch.VertexFactory = &Section.VertexFactory;
		MeshBatch.Type = PT_TriangleList;
		MeshBatch.CastShadow = Section.ShouldRenderShadow();

		MeshBatch.LODIndex = Section.LODIndex;
	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		MeshBatch.VisualizeLODIndex = Section.LODIndex;
	#endif

		MeshBatch.SegmentIndex = SectionId;

		MeshBatch.bDitheredLODTransition = !bForRayTracing && !IsMovable() && MaterialRenderProxy->GetMaterialInterface()->IsDitheredLODTransition();
		MeshBatch.bWireframe = !bForRayTracing && WireframeMaterial != nullptr;

		MeshBatch.MaterialRenderProxy = MeshBatch.bWireframe ? WireframeMaterial : MaterialRenderProxy;
		MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();

		MeshBatch.DepthPriorityGroup = SDPG_World;
		MeshBatch.bCanApplyViewModeOverrides = false;
		
		FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
		BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
		BatchElement.IndexBuffer = &Section.IndexBuffer;
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = Section.NumPrimitives;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = Section.MaxVertex;
	}

	//~ Begin FPrimativeSceneProxy
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);

		bool bForceDynamicPath = IsRichView(*View->Family) || IsSelected() || View->Family->EngineShowFlags.Wireframe;
		Result.bStaticRelevance = !bForceDynamicPath && bShouldRenderStatic;
		Result.bDynamicRelevance = !Result.bStaticRelevance;

		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;		
	}

	bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	uint32 GetMemoryFootprint(void) const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{
		SCOPE_CYCLE_COUNTER(STAT_SimpleMeshSceneProxy_DrawStaticMeshElements);

		for (int SectionIndex = 0, SectionIndexCnt = Sections.Num(); SectionIndex < SectionIndexCnt; SectionIndex++)
		{
			auto &Section = *(Sections[SectionIndex]);
						
			if (Section.ShouldRenderStaticPath() && bShouldRenderStatic)
			{
				FMeshBatch MeshBatch;
				MeshBatch.LODIndex = Section.LODIndex;
				MeshBatch.SegmentIndex = SectionIndex;

				CreateMeshBatch(MeshBatch, Section, SectionIndex, nullptr, false);
				PDI->DrawMesh(MeshBatch, 1.f);
			}
		}
	}

	void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		
		SCOPE_CYCLE_COUNTER(STAT_SimpleMeshSceneProxy_GetDynamicMeshElements);

		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe) // Wireframe Color
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr, FLinearColor(0, 0.5f, 1.f));

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			const FSceneView* View = Views[ViewIndex];
			bool bForceDynamicPath = !bShouldRenderStatic || IsRichView(*Views[ViewIndex]->Family) || Views[ViewIndex]->Family->EngineShowFlags.Wireframe || IsSelected();
			
			if (IsShown(View) && (VisibilityMap & (1 << ViewIndex)))
			{
				FFrozenSceneViewMatricesGuard FrozenMatricesGuard(*const_cast<FSceneView*>(Views[ViewIndex]));
				if (bForceDynamicPath)
				{
					for (int SectionIndex = 0, SectionIndexCnt = Sections.Num(); SectionIndex < SectionIndexCnt; SectionIndex++)
					{
						auto &Section = *(Sections[SectionIndex]);
						if (Section.ShouldRenderDynamicPath() || bForceDynamicPath)
						{
							FMeshBatch& MeshBatch = Collector.AllocateMesh();
							CreateMeshBatch(MeshBatch, Section, SectionIndex, WireframeMaterialInstance, false);
							MeshBatch.bDitheredLODTransition = false;
							Collector.AddMesh(ViewIndex, MeshBatch);
						}
					}
				}
			}
		}
		// Draw bounds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				// Draw simple collision as wireframe if 'show collision', and collision is enabled, and we are not using the complex as the simple
				/*if (ViewFamily.EngineShowFlags.Collision && IsCollisionEnabled() && BodySetup && BodySetup->GetCollisionTraceFlag() != ECollisionTraceFlag::CTF_UseComplexAsSimple)
				{
					FTransform GeomTransform(GetLocalToWorld());
					BodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(FColor(157, 149, 223, 255), IsSelected(), IsHovered()).ToFColor(true), NULL, false, false, DrawsVelocity(), ViewIndex, Collector);
				}*/
				// Render bounds
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			}
		}
#endif
	}
	
#if RHI_RAYTRACING
	bool IsRayTracingRelevant() const override
	{ 
		return true; 
	}
	bool IsRayTracingStaticRelevant() const override
	{ 
		return false; 
	}

	void GetDynamicRayTracingInstances(struct FRayTracingMaterialGatheringContext& Context, TArray<struct FRayTracingInstance>& OutRayTracingInstances) override
	{
		SCOPE_CYCLE_COUNTER(STAT_SimpleMeshSceneProxy_GetDynamicRayTracingInstances);

		for (int SectionIndex = 0, SectionIndexCnt = Sections.Num(); SectionIndex < SectionIndexCnt; SectionIndex++)
		{
			auto &Section = *(Sections[SectionIndex]);
						
			if (Section.ShouldRenderDynamicPathRayTracing())
			{
				FRayTracingGeometry* SectionRayTracingGeometry = &Section.RayTracingGeometry;

				if (SectionRayTracingGeometry->RayTracingGeometryRHI->IsValid())
				{
					check(SectionRayTracingGeometry->Initializer.TotalPrimitiveCount > 0);
					check(SectionRayTracingGeometry->Initializer.IndexBuffer.IsValid());

					FRayTracingInstance RayTracingInstance;
					RayTracingInstance.Geometry = SectionRayTracingGeometry;
					RayTracingInstance.InstanceTransforms.Add(GetLocalToWorld());

					FMeshBatch MeshBatch;
					CreateMeshBatch(MeshBatch, Section, SectionIndex, nullptr, true);
					MeshBatch.CastRayTracedShadow = IsShadowCast(Context.ReferenceView);
					RayTracingInstance.Materials.Add(MeshBatch);

					//RayTracingInstance.BuildInstanceMaskAndFlags();
					OutRayTracingInstances.Add(RayTracingInstance);
				}
			}
		}
	}
#endif // RHI_RAYTRACING
	//~ Begin FPrimativeSceneProxy
};