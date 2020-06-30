
#pragma once

#include "material.hpp"
#include <map>

#include <DiligentCore/Graphics/GraphicsEngine/interface/BlendState.h>

namespace dg {

class UnlitMaterial : public Material<material::Diffuse>
{
public:
	DG_PTR(UnlitMaterial)

	UnlitMaterial(IRenderDevice* device);

	virtual void initialize(IRenderDevice* device) override;
	virtual void setupPSODesc(PipelineStateDesc& desc) override;
    virtual void bindPSO(IPipelineState* pso) override;
    virtual void bindSRB(IShaderResourceBinding* srb) override;
	virtual void prepareForRender(IDeviceContext* context) override;

	void setBlendDesc(const RenderTargetBlendDesc& desc);

private:

	struct MaterialVS;

	RenderTargetBlendDesc _blend_desc;

private:

	static std::map<std::pair<IRenderDevice*, int>, std::weak_ptr<ShaderProgram>> _shared_shader_programs;
};


}
