#include "CustomVertexFactory.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"

/*
 * FCustomVertexFactoryShaderParameters
 * 
 * We can bind shader parameters here
 * There's two types of shader parameters: FShaderPrameter and FShaderResourcePramater
 * We can use the first to pass parameters like floats, integers, arrays
 * W can use the second to pass shader resources bindings, for example Structured Buffer, texture, samplerstate, etc
 * Actually that's how manual fetch is implmented; for each of the Vertex Buffers of the stream components, an SRV is created
 * That SRV can bound as a shader resource parameter and you can fetch the buffers using the SV_VertexID
*/


IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FCustomVertexFactoryParameters, "CustomVF");

class FCustomVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FCustomVertexFactoryShaderParameters, NonVirtual);

	void Bind(const FShaderParameterMap& ParameterMap)
	{
	}

	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const FSceneView* View,
		const class FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* InVertexFactory,
		const FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		FCustomVertexFactory* VertexFactory = (FCustomVertexFactory*)InVertexFactory;
		ShaderBindings.Add(Shader->GetUniformBufferParameter<FCustomVertexFactoryParameters>(),
		                   VertexFactory->GetCustomPrimitiveVertexFactoryUniformBuffer());
	}
};

IMPLEMENT_TYPE_LAYOUT(FCustomVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FCustomVertexFactory, SF_Vertex, FCustomVertexFactoryShaderParameters);

void FCustomVertexFactory::InitRHI(FRHICommandListBase& RHICmdList)
{
	// No Stream Should Currently Exist
	check(Streams.Num() == 0);

	//查看 StaticMesh 的 RenderData 的 LODResources[0] 的 VertexBuffer 可知
	/*
	 *PositionVertexBuffer.Stride = 12;
	 * TangentStride = 8; Offset = 4;	//StaticMeshVertexBuffer.TangentsData 可看具体数据
	 * TexcoordStride = 4;				//StaticMeshVertexBuffer.TexcoordData 可看具体数据
	 */

	// 对应Shader中 ATTRIBUTE0
	// Position stream for per vertex local position 
	FVertexStream PositionVertexStream;
	PositionVertexStream.VertexBuffer = VertexBuffer;
	PositionVertexStream.Stride = 12;
	//StaticMesh.RenderData.LODResources[0].VertexBuffers。PositionVertexBuffer.GetStride() //可以查看这个
	PositionVertexStream.Offset = 0;
	PositionVertexStream.VertexStreamUsage = EVertexStreamUsage::Default;

	// 对应shader中的 ATTRIBUTE1
	FVertexStream TangentXVertexStream;
	TangentXVertexStream.VertexBuffer = TangentVertexBuffer;
	TangentXVertexStream.Stride = 8;
	// StaticMesh.RenderData.LODResources[0].VertexBuffers.StaticMeshVertexBuffer 的 TangentStrider = 8 可以查看这个
	TangentXVertexStream.Offset = 0;
	TangentXVertexStream.VertexStreamUsage = EVertexStreamUsage::Default;

	FVertexStream TangentZVertexStream;
	TangentZVertexStream.VertexBuffer = TangentVertexBuffer;
	TangentZVertexStream.Stride = 8;
	// StaticMesh.RenderData.LODResources[0].VertexBuffers.StaticMeshVertexBuffer 的 TangentStrider = 8 可以查看这个
	TangentZVertexStream.Offset = 4; //前面4个是 x.后面4个是 z，所以要 offset 4
	TangentZVertexStream.VertexStreamUsage = EVertexStreamUsage::Default;

	const FVertexElement VertexPositionElement(Streams.Add(PositionVertexStream), 0, VET_Float3, 0,
	                                           PositionVertexStream.Stride, false);
	const FVertexElement VertexTangentXElement(Streams.Add(TangentXVertexStream), 0, VET_PackedNormal, 1,
	                                           TangentXVertexStream.Stride, false);
	const FVertexElement VertexTangentZElement(Streams.Add(TangentZVertexStream), 0, VET_PackedNormal, 2,
	                                           TangentZVertexStream.Stride, false);

	//The vertex declaration element lists (Nothing but an array of FVertexElement)
	FVertexDeclarationElementList Elements;
	Elements.Add(VertexPositionElement); //ATTRIBUTE0
	Elements.Add(VertexTangentXElement); //ATTRIBUTE1	
	Elements.Add(VertexTangentZElement); //ATTRIBUTE2

	//简化写法
	/*Elements.Add(AccessStreamComponent(FVertexStreamComponent(VertexBuffer, 0, 12, VET_Float3), 0));
	Elements.Add(AccessStreamComponent(FVertexStreamComponent(TangentVertexBuffer, 0, 8, VET_PackedNormal), 1));
	Elements.Add(AccessStreamComponent(FVertexStreamComponent(TangentVertexBuffer, 4, 8, VET_PackedNormal), 2));*/

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

void FCustomVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
                                                        FShaderCompilerEnvironment& OutEnvironment)
{
	FVertexFactory::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

void FCustomVertexFactory::SetParameters(FCustomVertexFactoryParameters& InUniformParameters)
{
	UniformBuffer = TUniformBufferRef<FCustomVertexFactoryParameters>::CreateUniformBufferImmediate(
		InUniformParameters, UniformBuffer_MultiFrame);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FCustomVertexFactory, "/CustomMesh/CustomVertexFactory.ush"
                              , EVertexFactoryFlags::SupportsManualVertexFetch
                              | EVertexFactoryFlags::SupportsDynamicLighting
                              | EVertexFactoryFlags::UsedWithMaterials);

/*
 * FCustomVertexFactory
 */
