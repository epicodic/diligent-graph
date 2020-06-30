#pragma once

#include <vector>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RasterizerState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DepthStencilState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>

#include <dg/material/material.hpp>

#include <dg/scene/object.hpp>
#include <dg/scene/render_order.hpp>

namespace dg {




class SceneManager;
class Renderable : public Object
{

public:

	Renderable() : Object(type_id<Renderable>()) {}

	virtual ~Renderable() = default;

public:

    void setRenderOrder(RenderOrder order)
    {
        _render_order = order;
    }

protected:

	void setPsoNeedsUpdate() { _pso_needs_update = true; }

public:

	friend class SceneManager;

	IMaterial::Ptr _material;

	RenderOrder                _render_order;
    RasterizerStateDesc        _rasterizerDesc;
    DepthStencilStateDesc      _depthStencilDesc;
	RefCntAutoPtr<IBuffer>     _vertexBuffer;
	RefCntAutoPtr<IBuffer>     _indexBuffer;
	std::uint32_t              _indexCount = 0;
	std::vector<LayoutElement> _inputLayout;
	PRIMITIVE_TOPOLOGY         _primitiveTopology = PRIMITIVE_TOPOLOGY_UNDEFINED;

private:
	// managed by SceneManager
	bool _pso_needs_update = true;
	IPipelineState* _pso = nullptr;
	RefCntAutoPtr<IShaderResourceBinding> _srb;
};


}
