
#include "CustomPrimitiveComponent.h"

#include "CustomPrimitiveSceneProxy.h"
#include "DataDrivenShaderPlatformInfo.h"

void UCustomPrimitiveComponent::SetIsVisible(bool bNewVisible)
{
	bIsVisible = bNewVisible;
}

FPrimitiveSceneProxy* UCustomPrimitiveComponent::CreateSceneProxy()
{
	if(RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
	{
		SceneProxy = new FCustomPrimitiveSceneProxy(this);
		return SceneProxy;
	}
	return nullptr;
}

void UCustomPrimitiveComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	if(Material != nullptr)
	{
		OutMaterials.Add(Material);
	}
}

int32 UCustomPrimitiveComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UCustomPrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if(StaticMesh != nullptr)
	{
		// Graphics bounds.
		FBoxSphereBounds NewBounds = StaticMesh->GetBounds().TransformBy(LocalToWorld);
		NewBounds.BoxExtent *= BoundsScale;
		NewBounds.SphereRadius *= BoundsScale;

		return NewBounds;
	}
	else
	{
		return FBoxSphereBounds(LocalToWorld.GetLocation(),FVector::ZeroVector , 0.f );
	}
}