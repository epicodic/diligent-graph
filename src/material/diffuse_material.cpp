
#include <dg/material/diffuse_material.hpp>
#include <dg/material/common_constants.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>

namespace dg {

struct DiffuseMaterial::MaterialVS
{
	Color color;
};

static const char* g_diffuse_material_vs =
DG_COMMON_CONSTANTS_VS_CODE
R"===(
cbuffer Material
{
    float4 g_color;
};

struct VSInput
{
    float3 Pos   : ATTRIB0;
    float3 normal : ATTRIB1;
//    float4 Color : ATTRIB2;
};

struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float4 Color : COLOR0; 
};

void main(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos   = mul(g_worldViewProj,  float4(VSIn.Pos,1.0));

    float3 N = mul(float3x3(g_worldView), VSIn.normal);

    float3 lightdir = float3(-1,-1,-1);

    float3 L = mul(float3x3(g_view), -normalize(lightdir)); 
    float NdotL = dot(N, L);

    PSIn.Color = float4(0.0); 
    if(NdotL > 0.f)     //compute diffuse color       Out.Color += NdotL * lights[i].vDiffuse
        PSIn.Color += g_color * NdotL; //VSIn.Color;

    //PSIn.Color = min(float4(1.0), PSIn.Color); 
}
)===";


static const char* g_diffuse_material_ps =
R"===(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float4 Color : COLOR0; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = PSIn.Color; 
}
)===";

std::map<IRenderDevice*, std::weak_ptr<ShaderProgram>> DiffuseMaterial::shared_shader_programs_;

DiffuseMaterial::DiffuseMaterial(IRenderDevice* device)
{
	initialize(device);
}

void DiffuseMaterial::initialize(IRenderDevice* device)
{
	std::weak_ptr<ShaderProgram>& shared_shader_program = shared_shader_programs_[device];

	if(shared_shader_program.expired())
	{
		shader_program_ = std::make_shared<ShaderProgram>();
		shader_program_->setShaders(device, "DiffuseMaterial_shader", g_diffuse_material_vs, g_diffuse_material_ps);
		shader_program_->addConstant<CommonConstantsVS>(device, "CommonConstantsVS");
		shader_program_->addConstant<MaterialVS>(device, "Material");
		shared_shader_program = shader_program_;
	}
	else
		shader_program_ = shared_shader_program.lock();

}

void DiffuseMaterial::setupPSODesc(PipelineStateDesc& desc)
{
	DG_ASSERT(_shader_program);

	desc.GraphicsPipeline.pVS = shader_program_->getVertexShader();
	desc.GraphicsPipeline.pPS = shader_program_->getPixelShader();
}

void DiffuseMaterial::bindPSO(IPipelineState* pso)
{
	DG_ASSERT(_shader_program);

	shader_program_->bind(pso);
}

void DiffuseMaterial::bindSRB(IShaderResourceBinding* srb)
{

}

void DiffuseMaterial::prepareForRender(IDeviceContext* context)
{
	auto material = shader_program_->mapConstant<MaterialVS>(context, "Material");
	material->color = color;
}

}
