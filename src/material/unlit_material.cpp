
#include <dg/material/unlit_material.hpp>
#include <dg/material/common_constants.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>

namespace dg {

struct UnlitMaterial::MaterialVS
{
	Color color;
	float opacity;
};

static const char* g_unlit_material_vs =
DG_COMMON_CONSTANTS_VS_CODE
R"===(
cbuffer Material
{
    float4 g_color;
    float  g_opacity;
};

struct VSInput
{
    float3 Pos   : ATTRIB0;
    float4 Color : ATTRIB2;
    float2 UV    : ATTRIB3;
};

struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float4 Color : COLOR0; 
    float2 UV    : TEX_COORD; 
};

void main(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos   = mul(g_worldViewProj,  float4(VSIn.Pos,1.0));
    PSIn.Color = g_color * VSIn.Color;
    PSIn.Color.a = PSIn.Color.a * g_opacity;
    PSIn.UV  = VSIn.UV;
}
)===";


static const char* g_unlit_material_ps =
R"===(

Texture2D    g_Texture;
SamplerState g_Texture_sampler;

struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float4 Color : COLOR0; 
    float2 UV    : TEX_COORD;
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{

#if USE_TEXTURE
    PSOut.Color = PSIn.Color * g_Texture.Sample(g_Texture_sampler, PSIn.UV);
#else 
    PSOut.Color = PSIn.Color; 
#endif

}
)===";

std::map<std::pair<IRenderDevice*, int>, std::weak_ptr<ShaderProgram>> UnlitMaterial::shared_shader_programs;

UnlitMaterial::UnlitMaterial(IRenderDevice* device)
{
	initialize(device);
}

void UnlitMaterial::initialize(IRenderDevice* device)
{
	blend_desc_.BlendEnable = true;
	blend_desc_.RenderTargetWriteMask = COLOR_MASK_ALL;
	blend_desc_.SrcBlend       = BLEND_FACTOR_SRC_ALPHA;
	blend_desc_.DestBlend      = BLEND_FACTOR_INV_SRC_ALPHA;
	blend_desc_.BlendOp        = BLEND_OPERATION_ADD;
	blend_desc_.SrcBlendAlpha  = BLEND_FACTOR_SRC_ALPHA;
	blend_desc_.DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
	blend_desc_.BlendOpAlpha   = BLEND_OPERATION_ADD;

    std::pair<IRenderDevice*, int> key(device, texture ? 1 : 0);
	std::weak_ptr<ShaderProgram>& shared_shader_program = shared_shader_programs[key];

	if(shared_shader_program.expired())
	{
		shader_program_ = std::make_shared<ShaderProgram>();

		ShaderProgram::MacroDefinitions macros;
		if(texture)
		    macros.push_back(std::make_pair("USE_TEXTURE", "1"));

		shader_program_->setShaders(device, "UnlitMaterial_shader", g_unlit_material_vs, g_unlit_material_ps, macros);
		shader_program_->addConstant<CommonConstantsVS>(device, "CommonConstantsVS");
		shader_program_->addConstant<MaterialVS>(device, "Material");
		shared_shader_program = shader_program_;
	}
	else
		shader_program_ = shared_shader_program.lock();
}

void UnlitMaterial::setupPSODesc(PipelineStateDesc& desc)
{
	DG_ASSERT(_shader_program);

	desc.GraphicsPipeline.pVS = shader_program_->getVertexShader();
	desc.GraphicsPipeline.pPS = shader_program_->getPixelShader();


    desc.GraphicsPipeline.RasterizerDesc.CullMode = (CULL_MODE) cull_mode;

	desc.GraphicsPipeline.BlendDesc.IndependentBlendEnable = False;
	RenderTargetBlendDesc& rt0 = desc.GraphicsPipeline.BlendDesc.RenderTargets[0];
	rt0 = blend_desc_;

    // Shader variables should typically be mutable, which means they are expected
    // to change on a per-instance basis
    static ShaderResourceVariableDesc vars[] =
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    desc.ResourceLayout.Variables    = vars;
    desc.ResourceLayout.NumVariables = _countof(vars);

    // Define static sampler for g_Texture. Static samplers should be used whenever possible
    static SamplerDesc sam_linear_clamp_desc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };
    static StaticSamplerDesc static_samplers[] =
    {
        {SHADER_TYPE_PIXEL, "g_Texture", sam_linear_clamp_desc}
    };

    desc.ResourceLayout.StaticSamplers    = static_samplers;
    desc.ResourceLayout.NumStaticSamplers =  _countof(static_samplers);

}

void UnlitMaterial::bindPSO(IPipelineState* pso)
{
	DG_ASSERT(_shader_program);

	shader_program_->bind(pso);
}

void UnlitMaterial::bindSRB(IShaderResourceBinding* srb)
{
	if(texture)
	{
        // Get shader resource view from the texture
        static dg::RefCntAutoPtr<ITextureView> texture_srv;
        texture_srv = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        // Set texture SRV in the SRB
        auto var = srb->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture");
        var->Set(texture_srv);
	}
}

void UnlitMaterial::prepareForRender(IDeviceContext* context)
{
	auto material = shader_program_->mapConstant<MaterialVS>(context, "Material");
	material->color = color;
	material->opacity = opacity;
}


void UnlitMaterial::setBlendDesc(const RenderTargetBlendDesc& desc)
{
	blend_desc_ = desc;
}

}
