
#include <dg/material/unlit_material.hpp>
#include <dg/material/common_constants.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>

namespace dg {

struct UnlitMaterial::MaterialVS
{
	Color color;
	float opacity;
};

static const char* UnlitMaterialVS =
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


static const char* UnlitMaterialPS =
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

std::map<std::pair<IRenderDevice*, int>, std::weak_ptr<ShaderProgram>> UnlitMaterial::_shared_shader_programs;

UnlitMaterial::UnlitMaterial(IRenderDevice* device)
{
	initialize(device);
}

void UnlitMaterial::initialize(IRenderDevice* device)
{
	_blend_desc.BlendEnable = true;
	_blend_desc.RenderTargetWriteMask = COLOR_MASK_ALL;
	_blend_desc.SrcBlend       = BLEND_FACTOR_SRC_ALPHA;
	_blend_desc.DestBlend      = BLEND_FACTOR_INV_SRC_ALPHA;
	_blend_desc.BlendOp        = BLEND_OPERATION_ADD;
	_blend_desc.SrcBlendAlpha  = BLEND_FACTOR_SRC_ALPHA;
	_blend_desc.DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
	_blend_desc.BlendOpAlpha   = BLEND_OPERATION_ADD;

    std::pair<IRenderDevice*, int> key(device, texture ? 1 : 0);
	std::weak_ptr<ShaderProgram>& shared_shader_program = _shared_shader_programs[key];

	if(shared_shader_program.expired())
	{
		_shader_program = std::make_shared<ShaderProgram>();

		ShaderProgram::MacroDefinitions macros;
		if(texture)
		    macros.push_back(std::make_pair("USE_TEXTURE", "1"));

		_shader_program->setShaders(device, "UnlitMaterial_shader", UnlitMaterialVS, UnlitMaterialPS, macros);
		_shader_program->addConstant<CommonConstantsVS>(device, "CommonConstantsVS");
		_shader_program->addConstant<MaterialVS>(device, "Material");
		shared_shader_program = _shader_program;
	}
	else
		_shader_program = shared_shader_program.lock();
}

void UnlitMaterial::setupPSODesc(PipelineStateDesc& desc)
{
	DG_ASSERT(_shader_program);

	desc.GraphicsPipeline.pVS = _shader_program->getVertexShader();
	desc.GraphicsPipeline.pPS = _shader_program->getPixelShader();

	desc.GraphicsPipeline.BlendDesc.IndependentBlendEnable = False;
	RenderTargetBlendDesc& rt0 = desc.GraphicsPipeline.BlendDesc.RenderTargets[0];
	rt0 = _blend_desc;

    // Shader variables should typically be mutable, which means they are expected
    // to change on a per-instance basis
    static ShaderResourceVariableDesc Vars[] =
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    desc.ResourceLayout.Variables    = Vars;
    desc.ResourceLayout.NumVariables = _countof(Vars);

    // Define static sampler for g_Texture. Static samplers should be used whenever possible
    static SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };
    static StaticSamplerDesc StaticSamplers[] =
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
    };

    desc.ResourceLayout.StaticSamplers    = StaticSamplers;
    desc.ResourceLayout.NumStaticSamplers =  _countof(StaticSamplers);

}

void UnlitMaterial::bindPSO(IPipelineState* pso)
{
	DG_ASSERT(_shader_program);

	_shader_program->bind(pso);
}

void UnlitMaterial::bindSRB(IShaderResourceBinding* srb)
{
	if(texture)
	{
        // Get shader resource view from the texture
        static dg::RefCntAutoPtr<ITextureView> textureSRV;
        textureSRV = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        // Set texture SRV in the SRB
        auto var = srb->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture");
        var->Set(textureSRV);
	}
}

void UnlitMaterial::prepareForRender(IDeviceContext* context)
{
	auto material = _shader_program->mapConstant<MaterialVS>(context, "Material");
	material->color = color;
	material->opacity = opacity;
}


void UnlitMaterial::setBlendDesc(const RenderTargetBlendDesc& desc)
{
	_blend_desc = desc;
}

}
