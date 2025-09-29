#pragma once

#include "CustomPrimitiveComponent.h"
#include "CustomVertexFactory.h"

class FCustomPrimitiveSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	
	FCustomPrimitiveSceneProxy(UCustomPrimitiveComponent* Component);
	virtual ~FCustomPrimitiveSceneProxy();

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

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
	virtual void CreateRenderThreadResources(FRHICommandListBase& RHICmdList) override;	//这里会调用到 FCustomVertexFactory::InitRHI

	void UpdateStaticMesh(UStaticMesh* InStaticMesh);

private:
	bool bIsVisible;

	/* 来自于UCustomPrimitiveComponent */
	UMaterialInterface* Material;

	UStaticMesh* StaticMesh;
	//存储需要在那些参与那些 Pass 绘制 ，评估渲染相关性
	FMaterialRelevance MaterialRelevance;
	FCustomVertexFactory VertexFactory;
};
