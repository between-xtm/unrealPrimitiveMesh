#include "CustomVertexFactory.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "MeshMaterialShader.h"

void FCustomVertexFactory::InitRHI(FRHICommandListBase& RHICmdList)
{
	// FVertexStream Stream;
	// No Stream Should Currently Exist
	check(Streams.Num() == 0);

	//The vertex declaration element lists (Nothing but an array of FVertexElement)
	FVertexDeclarationElementList Elements;
	
	// 对应Shader中 ATTRIBUTE0
	// Position stream for per vertex local position 
	FVertexStream PositionVertexStream;
	PositionVertexStream.VertexBuffer = VertexBuffer;
	PositionVertexStream.Stride = sizeof(FVector)/2;
	PositionVertexStream.Offset = 0;
	PositionVertexStream.VertexStreamUsage = EVertexStreamUsage::Default;

	FVertexElement VertexPositionElement(Streams.Add(PositionVertexStream), 0, VET_Float3, 0, PositionVertexStream.Stride, false);

	
	Elements.Add(VertexPositionElement);

	InitDeclaration(Elements);
}
void FCustomVertexFactory::ReleaseRHI()
{
	UniformBuffer.SafeRelease();
	FVertexFactory::ReleaseRHI();
}
bool FCustomVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return RHISupportsManualVertexFetch(Parameters.Platform);
}
void FCustomVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FVertexFactory::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FCustomVertexFactory , "/CustomMesh/CustomVertexFactory.ush" 
	/*,EVertexFactoryFlags::UsedWithMaterials*/,EVertexFactoryFlags::SupportsManualVertexFetch);