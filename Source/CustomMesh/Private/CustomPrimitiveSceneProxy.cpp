#include "CustomPrimitiveSceneProxy.h"
#include "CustomPrimitiveComponent.h"
#include "Materials/MaterialRenderProxy.h"

FCustomPrimitiveSceneProxy::FCustomPrimitiveSceneProxy(UCustomPrimitiveComponent* Component)
	:FPrimitiveSceneProxy(Component)
	,bIsVisible(Component->bIsVisible)
	,Material(Component->Material)
	,StaticMesh(Component->StaticMesh)
	,MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	,VertexFactory(GetScene().GetFeatureLevel() , "FCustomVertexFactory")
{
	if(StaticMesh)
	{
		UpdateStaticMesh(StaticMesh);	//UpdateStaticMesh 会设置 VertexFactory.VertexBuffer , 从而影响Shader中的 ATTBUTE0 的值
	}
}

FCustomPrimitiveSceneProxy::~FCustomPrimitiveSceneProxy()
{
	VertexFactory.ReleaseResource();
}

FPrimitiveViewRelevance FCustomPrimitiveSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View) && bIsVisible;;
	Result.bShadowRelevance = false;
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = true;
	Result.bUsesLightingChannels = false;
	Result.bRenderCustomDepth = false;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

void FCustomPrimitiveSceneProxy::CreateRenderThreadResources(FRHICommandListBase& RHICmdList)
{
	VertexFactory.InitResource(RHICmdList);	//这里会调用到 FCustomVertexFactory::InitRHI
}

void FCustomPrimitiveSceneProxy::UpdateStaticMesh(UStaticMesh* InStaticMesh)
{
	if(InStaticMesh != nullptr)
	{
		StaticMesh = InStaticMesh;
		VertexFactory.VertexBuffer = &StaticMesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
	}
}

void FCustomPrimitiveSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	if(!bIsVisible)
	{
		return;
	}
	if(!StaticMesh)
	{
		return;
	}

	const bool bIsWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	FMaterialRenderProxy* MaterialProxy = Material->GetRenderProxy();
	if(bIsWireframe)
	{
		FMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FColor::Cyan);
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		MaterialProxy = WireframeMaterialInstance;
	}
	//Nothing to render with
	if(MaterialProxy == nullptr)
	{
		return;
	}
	for(int32 ViewIndex = 0 ; ViewIndex < Views.Num() ; ViewIndex++)
	{
		if(VisibilityMap & (1 << ViewIndex))
		{
			if(bIsWireframe)
			{
				RenderBounds(Collector.GetPDI(ViewIndex) , ViewFamily.EngineShowFlags , GetBounds() , IsSelectable());
			}

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, false, AlwaysHasVelocity());

			
			//Create a mesh batch for this chunk 
			FMeshBatch& MeshBatch = Collector.AllocateMesh();
			MeshBatch.CastShadow = false;
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
			BatchElement.MaxVertexIndex = StaticMesh->GetRenderData()->LODResources[0].IndexBuffer.GetNumIndices()-1;
			BatchElement.MinVertexIndex = 0;
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;//&GIdentityPrimitiveUniformBuffer;
			
			
			MeshBatch.bCanApplyViewModeOverrides = false;
			Collector.AddMesh(ViewIndex , MeshBatch);

		}
	}
	
}