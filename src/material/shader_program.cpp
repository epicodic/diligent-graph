#include <vector>
#include <dg/material/shader_program.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>

namespace dg {

ShaderProgram::ShaderProgram(IRenderDevice* device,
                             const std::string& name, const std::string& vs_code, const std::string& ps_code)
{
    setShaders(device, name, vs_code, ps_code);
}

void ShaderProgram::setShaders(IRenderDevice* device, const std::string& name,
                               const std::string& vs_code, const std::string& ps_code,
                               const MacroDefinitions& macros)
{
    ShaderCreateInfo shader_info;
    shader_info.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    shader_info.UseCombinedTextureSamplers = true;

    // Create a vertex shader
    shader_info.Desc.ShaderType = SHADER_TYPE_VERTEX;
    shader_info.EntryPoint      = "main";
    shader_info.Desc.Name       = (name + "_vs").c_str();
    shader_info.Source          = vs_code.c_str();


    std::vector<ShaderMacro> macros_vec;
    if(!macros.empty())
    {
        for(const auto& p : macros)
            macros_vec.push_back(ShaderMacro(p.first.c_str(),p.second.c_str()));
        macros_vec.push_back(ShaderMacro(nullptr,nullptr));

        shader_info.Macros = macros_vec.data();
    }

    device->CreateShader(shader_info, &vertex_shader_);

    // Create a pixel shader
    shader_info.Desc.ShaderType = SHADER_TYPE_PIXEL;
    shader_info.EntryPoint      = "main";
    shader_info.Desc.Name       = (name + "_ps").c_str();
    shader_info.Source          = ps_code.c_str();
    device->CreateShader(shader_info, &pixel_shader_);
}

void ShaderProgram::bind(IPipelineState* pso)
{
    for(auto& p : constants_)
    {
        Constant& c = p.second;
        pso->GetStaticVariableByName(c.shader_type, c.name.c_str())->Set(c.buffer);
    }
}

void ShaderProgram::addConstant(IRenderDevice* device, const ConstantDef& def)
{
    Constant constant(def);
    BufferDesc desc;
    desc.Name           = constant.name.c_str();
    desc.uiSizeInBytes  = constant.size;
    desc.Usage          = USAGE_DYNAMIC;
    desc.BindFlags      = BIND_UNIFORM_BUFFER;
    desc.CPUAccessFlags = CPU_ACCESS_WRITE;
    device->CreateBuffer(desc, nullptr, &constant.buffer);

    constants_[constant.name] = constant;
}

}
