#include "CustomPrimitiveComponent.h"

#include "CustomPrimitiveSceneProxy.h"
// #include "RHIUtilities.h"
// #include "DataDrivenShaderPlatformInfo.h"
// #include "Engine/MapBuildDataRegistry.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Rendering/StaticLightingSystemInterface.h"


/*
 * FCustomPrimitiveSceneProxy 渲染线程资源，对应于UCustomPrimitiveComponent
 */

/*
 * FCustomPrimitiveSceneProxy
 */

void UCustomPrimitiveComponent::SetIsVisible(bool bNewVisible)
{
	bIsVisible = bNewVisible;
}

const FMeshMapBuildData* UCustomPrimitiveComponent::GetMeshMapBuildData()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ULevel* OwnerLevel = Owner->GetLevel();

#if WITH_EDITOR
		if (FStaticLightingSystemInterface::GetPrimitiveMeshMapBuildData(this))
		{
			return FStaticLightingSystemInterface::GetPrimitiveMeshMapBuildData(this);
		}
#endif

		if (OwnerLevel && OwnerLevel->OwningWorld)
		{
			ULevel* ActiveLightingScenario = OwnerLevel->OwningWorld->GetActiveLightingScenario();
			UMapBuildDataRegistry* MapBuildData = nullptr;

			if (ActiveLightingScenario && ActiveLightingScenario->MapBuildData)
			{
				MapBuildData = ActiveLightingScenario->MapBuildData;
			}
			else if (OwnerLevel->MapBuildData)
			{
				MapBuildData = OwnerLevel->MapBuildData;
			}

			if (MapBuildData)
			{
				return MapBuildData->GetMeshBuildData(MapBuildDataId);
			}
		}
	}

	return nullptr;
}

void UCustomPrimitiveComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// Create a new guid in case this is a newly create component
	// If not, this guid will be overwritten when serialized
	FPlatformMisc::CreateGuid(StateId);

	// Initialize MapBuildDataId to something unique
	MapBuildDataId = FGuid::NewGuid();
}

FPrimitiveSceneProxy* UCustomPrimitiveComponent::CreateSceneProxy()
{
	if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
	{
		SceneProxy = new FCustomPrimitiveSceneProxy(this);
		return SceneProxy;
	}
	return nullptr;
}

void UCustomPrimitiveComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials,
                                                 bool bGetDebugMaterials) const
{
	if (Material != nullptr)
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
	if (StaticMesh != nullptr)
	{
		// Graphics bounds.
		FBoxSphereBounds NewBounds = StaticMesh->GetBounds().TransformBy(LocalToWorld);
		NewBounds.BoxExtent *= BoundsScale;
		NewBounds.SphereRadius *= BoundsScale;

		return NewBounds;
	}
	return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f);
}
