#pragma once

#include "CoreMinimal.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "UObject/Object.h"
// 自定义 PrimitiveComponent 的 UniformBuffer结构 , 可以参考 LocalVertexFactory.h 的 FLocalVertexFactoryUniformShaderParameters

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FCustomVertexFactoryParameters,)
	SHADER_PARAMETER(FIntVector4, VertexFetch_Parameters)
	SHADER_PARAMETER_SRV(Buffer<float2>, VertexFetch_TexCoordBuffer)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/*
 * FCustomVertexFactory
 */
// class FCustomVertexFactoryParameters;
class FCustomVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FCustomVertexFactory);

	FCustomVertexFactory(
		ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName)
		: FVertexFactory(InFeatureLevel)
	{
	}

	// Construct Render Resources for this vertex factory
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	// release Render Resources for this vertex factory
	virtual void ReleaseRHI() override;

	//should we cache material`s shadertype on this platform with this vertex factory type?
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
	                                         FShaderCompilerEnvironment& OutEnvironment);

	void SetParameters(FCustomVertexFactoryParameters& InUniformParameters);

	const FUniformBufferRHIRef GetCustomPrimitiveVertexFactoryUniformBuffer() const
	{
		return UniformBuffer;
	}

public:
	FVertexBuffer* VertexBuffer = nullptr;
	FVertexBuffer* TangentVertexBuffer = nullptr;
	/* Buffers to read from */
	TUniformBufferRef<FCustomVertexFactoryParameters> UniformBuffer;
};
