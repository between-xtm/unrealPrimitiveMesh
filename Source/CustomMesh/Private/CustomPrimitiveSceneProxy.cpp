#include "CustomPrimitiveSceneProxy.h"
// #include "CustomPrimitiveComponent.h"
#include "CustomVertexFactory.h"
#include "Materials/MaterialRenderProxy.h"

/*
 * FCustomPrimitiveSceneProxy 渲染线程资源，对应于UCustomPrimitiveComponent
 */

FCustomPrimitiveSceneProxy::FCustomPrimitiveSceneProxy(UCustomPrimitiveComponent* Component)
	: FPrimitiveSceneProxy(Component)
	  , bIsVisible(Component->bIsVisible)
	  , ComponentLightInfo(nullptr)
	  , Material(Component->Material)
	  , StaticMesh(Component->StaticMesh)
	  , MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	  , VertexFactory(GetScene().GetFeatureLevel(), "FCustomVertexFactory")
{
	ComponentLightInfo = MakeUnique<FCustomPrimitiveLCI>(Component);
	check(ComponentLightInfo);
	if (StaticMesh)
	{
		UpdateStaticMesh(StaticMesh); //UpdateStaticMesh 会设置 VertexFactory.VertexBuffer , 从而影响Shader中的 ATTBUTE0 的值
	}
}

FCustomPrimitiveSceneProxy::~FCustomPrimitiveSceneProxy()
{
	VertexFactory.ReleaseResource();
}

FPrimitiveViewRelevance FCustomPrimitiveSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View) && bIsVisible;
	Result.bShadowRelevance = true;
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = true;
	Result.bUsesLightingChannels = false;
	Result.bRenderCustomDepth = false;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

void FCustomPrimitiveSceneProxy::GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic,
                                                   bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const
{
	FPrimitiveSceneProxy::GetLightRelevance(LightSceneProxy, bDynamic, bRelevant, bLightMapped, bShadowMapped);

	if (ComponentLightInfo)
	{
		ELightInteractionType InteractionType = ComponentLightInfo->GetInteraction(LightSceneProxy).GetType();

		if (InteractionType != LIT_CachedIrrelevant)
		{
			bRelevant = true;
		}
		if (InteractionType != LIT_CachedIrrelevant && InteractionType != LIT_CachedLightMap)
		{
			bLightMapped = true;
		}
		if (InteractionType != LIT_Dynamic)
		{
			bDynamic = false;
		}
		if (InteractionType != LIT_CachedSignedDistanceFieldShadowMap2D)
		{
			bShadowMapped = false;
		}
	}
	else
	{
		bRelevant = true;
		bLightMapped = false;
		bShadowMapped = false;
	}
}

FLightInteraction FCustomPrimitiveSceneProxy::FCustomPrimitiveLCI::GetInteraction(
	const FLightSceneProxy* LightSceneProxy) const
{
	// ask base class
	ELightInteractionType LightInteraction = GetStaticInteraction(LightSceneProxy, IrrelevantLights);
	if (LightInteraction == LIT_MAX)
	{
		return FLightInteraction(LightInteraction);
	}
	// use dynamic light doesn`t have static lighting
	return FLightInteraction::Dynamic();
}

//设置FCustomVertexFactoryParameters
void FCustomPrimitiveSceneProxy::CreateRenderThreadResources(FRHICommandListBase& RHICmdList)
{
	VertexFactory.InitResource(); //这里会调用到 FCustomVertexFactory::InitRHI
	FCustomVertexFactoryParameters UniformParameters;
	if (StaticMesh) //创建并填充FCustomVertexFactoryParamters
	{
		int32 ColorIndexMask = 0;
		const int32 NumTexCoords = StaticMesh->GetRenderData()->LODResources[0].GetNumTexCoords();
		const int32 LightMapCoordinateIndex = StaticMesh->LightMapCoordinateIndex;
		constexpr int32 EffectiveBaseVertexIndex = 0;
		//const int32 EffectivePreSkinBaseVertexIndex = RHISupportsAbsoluteVertexID(GMaxRHIShaderPlatform) ? 0 : PreSkinBaseVertexIndex;

		UniformParameters.VertexFetch_Parameters = {
			ColorIndexMask, NumTexCoords, LightMapCoordinateIndex, EffectiveBaseVertexIndex
		};
		UniformParameters.VertexFetch_TexCoordBuffer = StaticMesh->GetRenderData()->LODResources[0].VertexBuffers.
			StaticMeshVertexBuffer.GetTexCoordsSRV();
	}
	else
	{
		UniformParameters.VertexFetch_Parameters = {0, 0, 0, 0};
		UniformParameters.VertexFetch_TexCoordBuffer = GNullVertexBuffer.VertexBufferSRV;
	}
	VertexFactory.SetParameters(UniformParameters);
}

void FCustomPrimitiveSceneProxy::UpdateStaticMesh(UStaticMesh* InStaticMesh)
{
	if (InStaticMesh != nullptr)
	{
		StaticMesh = InStaticMesh;
		VertexFactory.VertexBuffer = &StaticMesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
		VertexFactory.TangentVertexBuffer = &StaticMesh->GetRenderData()->LODResources[0].VertexBuffers.
			StaticMeshVertexBuffer.TangentsVertexBuffer;
	}
}

void FCustomPrimitiveSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                                        const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
                                                        FMeshElementCollector& Collector) const
{
	if (!bIsVisible)
	{
		return;
	}
	if (!StaticMesh)
	{
		return;
	}

	const bool bIsWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	FMaterialRenderProxy* MaterialProxy = Material->GetRenderProxy();
	if (bIsWireframe)
	{
		FMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			FColor::Cyan);
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		MaterialProxy = WireframeMaterialInstance;
	}
	//Nothing to render with
	if (MaterialProxy == nullptr)
	{
		return;
	}
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			if (bIsWireframe)
			{
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelectable());
			}

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<
				FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(),
			                                  GetBounds(), GetLocalBounds(), true, false, AlwaysHasVelocity());


			//Create a mesh batch for this chunk 
			FMeshBatch& MeshBatch = Collector.AllocateMesh();
			MeshBatch.LCI = ComponentLightInfo.Get();
			MeshBatch.CastShadow = true;
			MeshBatch.bUseAsOccluder = false;
			MeshBatch.VertexFactory = &VertexFactory;
			MeshBatch.MaterialRenderProxy = MaterialProxy;
			MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
			MeshBatch.DepthPriorityGroup = SDPG_World;

			// Set up index buffer
			MeshBatch.Type = PT_TriangleList;
			FMeshBatchElement& BatchElement = MeshBatch.Elements[0];

			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = StaticMesh->GetRenderData()->LODResources[0].IndexBuffer.GetNumIndices() / 3;
			BatchElement.IndexBuffer = &StaticMesh->GetRenderData()->LODResources[0].IndexBuffer;
			BatchElement.MaxVertexIndex = StaticMesh->GetRenderData()->LODResources[0].IndexBuffer.GetNumIndices() - 1;
			BatchElement.MinVertexIndex = 0;
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
			//&GIdentityPrimitiveUniformBuffer;


			MeshBatch.bCanApplyViewModeOverrides = false;
			Collector.AddMesh(ViewIndex, MeshBatch);
		}
	}
}
