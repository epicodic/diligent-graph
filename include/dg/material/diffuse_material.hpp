
#pragma once

#include "material.hpp"

namespace dg {

class DiffuseMaterial : public Material<material::Diffuse>
{
public:
	DG_PTR(DiffuseMaterial)

	DiffuseMaterial(IRenderDevice* device);

	virtual void initialize(IRenderDevice* device) override;
	virtual void setupPSODesc(PipelineStateDesc& desc) override;
    virtual void bindPSO(IPipelineState* pso) override;
    virtual void bindSRB(IShaderResourceBinding* srb) override;
	virtual void prepareForRender(IDeviceContext* context) override;

private:

	struct MaterialVS;

private:

	static std::map<IRenderDevice*, std::weak_ptr<ShaderProgram>> shared_shader_programs_;
};


}
