#pragma once
#include "CoreMinimal.h"

#include "CustomPrimitiveComponent.generated.h"

UCLASS(Blueprintable , meta = (BlueprintSpawnableComponent) , ClassGroup = Rendering)
class CUSTOMMESH_API UCustomPrimitiveComponent : public UMeshComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category="CustomPrimitiveMesh")
	bool bIsVisible;

	/*Change the Visibility Setting*/
	UFUNCTION(BlueprintCallable , Category="CustomPrimitiveMesh")
	void SetIsVisible(bool bNewVisible);

	UPROPERTY(EditAnywhere , Category="CustomPrimitiveMesh")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere , Category="CustomPrimitiveMesh")
	TObjectPtr<UMaterialInterface> Material;

private:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.
};