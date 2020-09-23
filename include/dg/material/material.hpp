#pragma once

#include <dg/core/common.hpp>
#include "DiligentCore/Graphics/GraphicsEngine/interface/RasterizerState.h"
#include "color.hpp"

#include "shader_program.hpp"

namespace dg {

class SceneManager;

class IMaterial
{
public:
	DG_DECL_PTR(IMaterial)

	virtual ~IMaterial() = default;

	virtual void initialize(IRenderDevice* device) = 0;
	virtual void setupPSODesc(PipelineStateDesc& desc) = 0;
	virtual void bindPSO(IPipelineState* pso) = 0;
	virtual void bindSRB(IShaderResourceBinding* srb) = 0;
	virtual void prepareForRender(IDeviceContext* context) = 0;

protected:
	friend class SceneManager;
	std::shared_ptr<ShaderProgram> _shader_program;
};

class BaseMaterial : public IMaterial
{
};

template<typename ... Components>
class Material : public BaseMaterial, public Components...
{
};


namespace material {

struct RasterizerParams
{

	enum class CullMode : Int8
	{
		Undefined = CULL_MODE_UNDEFINED,
		None = CULL_MODE_NONE,
		Front = CULL_MODE_FRONT,
		Back = CULL_MODE_BACK
	};

	CullMode cull_mode = CullMode::Back;

};


struct Diffuse
{
	/// Diffuse color
	Color color = Color(1.0f,1.0f,1.0f,1.0f);
	float opacity = 1;
	RefCntAutoPtr<ITexture> texture;
};


}


}

