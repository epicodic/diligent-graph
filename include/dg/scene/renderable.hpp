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
        render_order = order;
    }

protected:

	void setPsoNeedsUpdate() { pso_needs_update_ = true; }

public:

	friend class SceneManager;

	IMaterial::Ptr material;

	RenderOrder                render_order;
	RasterizerStateDesc        rasterizer_desc;
	DepthStencilStateDesc      depth_stencil_desc;
	RefCntAutoPtr<IBuffer>     vertex_buffer;
	RefCntAutoPtr<IBuffer>     index_buffer;
	std::uint32_t              index_count = 0;
	std::vector<LayoutElement> input_layout;
	PRIMITIVE_TOPOLOGY         primitive_topology = PRIMITIVE_TOPOLOGY_UNDEFINED;

private:
	// managed by SceneManager
	bool pso_needs_update_ = true;
	IPipelineState* pso_ = nullptr;
	RefCntAutoPtr<IShaderResourceBinding> srb_;
};


}
