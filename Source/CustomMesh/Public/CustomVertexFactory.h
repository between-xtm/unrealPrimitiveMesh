#pragma once

class FCustomVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FCustomVertexFactory);
public:
	FCustomVertexFactory(
		ERHIFeatureLevel::Type InFeatureLevel , const char* InDebugName)
		: FVertexFactory(InFeatureLevel)
	{
	}

	// Construct Render Resources for this vertex factory
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	// release Render Resources for this vertex factory
	virtual void ReleaseRHI() override;

	//should we cache material`s shadertype on this platform with this vertex factory type?
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

private:
	/* Buffers to read from */
	FUniformBufferRHIRef UniformBuffer;

public:
	FVertexBuffer* VertexBuffer = nullptr;
};