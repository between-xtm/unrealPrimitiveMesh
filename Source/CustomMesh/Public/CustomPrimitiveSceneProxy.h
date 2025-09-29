#pragma once

#include "CoreMinimal.h"
#include "CustomPrimitiveComponent.h"
#include "CustomVertexFactory.h"
#include "Engine/MapBuildDataRegistry.h"
#include "UObject/Object.h"

class FCustomPrimitiveSceneProxy final : public FPrimitiveSceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FCustomPrimitiveSceneProxy(UCustomPrimitiveComponent* Component);
	virtual ~FCustomPrimitiveSceneProxy() override;

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
	                                    uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant,
	                               bool& bLightMapped, bool& bShadowMapped) const override;

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	uint32 GetAllocatedSize() const
	{
		return FPrimitiveSceneProxy::GetAllocatedSize();
	}

	/*
	 *  当渲染线程添加这个Proxy到Scene的时候调用
	 *  这个函数允许生成渲染线程资源
	 *  在渲染线程调用
	 */
	virtual void CreateRenderThreadResources(FRHICommandListBase& RHICmdList) override;
	//这里会调用到 FCustomVertexFactory::InitRHI

	void UpdateStaticMesh(UStaticMesh* InStaticMesh);

private:
	bool bIsVisible;

	class FCustomPrimitiveLCI final : public FLightCacheInterface
	{
	public:
		FCustomPrimitiveLCI(UCustomPrimitiveComponent* InComponent)
			: FLightCacheInterface()
		{
			const FMeshMapBuildData* MapBuildData = InComponent->GetMeshMapBuildData();

			if (MapBuildData)
			{
				SetLightMap(MapBuildData->LightMap);
				SetShadowMap(MapBuildData->ShadowMap);
				SetResourceCluster(MapBuildData->ResourceCluster);
				IrrelevantLights = MapBuildData->IrrelevantLights;
			}
		}

		// FLightCacheInterface
		virtual FLightInteraction GetInteraction(const FLightSceneProxy* LightSceneProxy) const override;

	private:
		TArray<FGuid> IrrelevantLights;
	};

	// FLightCacheInterface
	TUniquePtr<FCustomPrimitiveLCI> ComponentLightInfo;

	/* 来自于UCustomPrimitiveComponent */
	UMaterialInterface* Material;

	UStaticMesh* StaticMesh;
	//存储需要在那些参与那些 Pass 绘制 ，评估渲染相关性
	FMaterialRelevance MaterialRelevance;
	FCustomVertexFactory VertexFactory;
};
