#pragma once

#include <dg/core/common.hpp>
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

template <typename ... Components>
struct MixinResolve;

template <typename ... Components>
class Material;

template<typename Component>
class Material<Component> : public BaseMaterial, public Component
{
	using ResolveT = MixinResolve<Component>;
};

template<>
class Material<> : public IMaterial
{
};

template<typename Component, typename ... Components>
class Material<Component, Components...> : public BaseMaterial, Component, Components...
{
	using ResolveT = MixinResolve<Component, Components...>;
};


namespace material {

struct Diffuse
{
	/// Diffuse color
	Color color = Color(1.0f,1.0f,1.0f,1.0f);
	float opacity = 1;
	RefCntAutoPtr<ITexture> texture;
};



}


}

