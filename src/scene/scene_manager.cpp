#include <dg/scene/scene_manager.hpp>

#include <dg/scene/node.hpp>
#include <dg/scene/renderable.hpp>
#include <dg/scene/raw_renderable.hpp>

#include <dg/internal/pso_hash.hpp>

#include <dg/core/conversion.hpp>
#include <dg/material/material.hpp>
#include <dg/material/common_constants.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>


#define FIRST_(a, ...) a
#define SECOND_(a, b, ...) b

#define FIRST(...) FIRST_(__VA_ARGS__,)
#define SECOND(...) SECOND_(__VA_ARGS__,)

#define EMPTY()

#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...) EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define DEFER1(m) m EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()

#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1

#define CAT(a,b) a ## b

#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()

#define BOOL(x) NOT(NOT(x))

#define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define COMMA ,

#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())
#define _END_OF_ARGUMENTS_() 0

#define MAP(m, first, ...)           \
  m(first)                           \
  IF_ELSE(HAS_ARGS(__VA_ARGS__))(    \
    COMMA DEFER2(_MAP)()(m, __VA_ARGS__)   \
  )(                                 \
    /* Do nothing, just terminate */ \
  )
#define _MAP() MAP

#define STRINGIZE(x) #x
#define MAGIC_MACRO(...) EVAL(MAP(STRINGIZE, __VA_ARGS__))


namespace dg {

SceneManager::SceneManager()
{
	root_ = Node::make();
	default_camera_node_ = getRoot()->createChild();
	default_camera_node_->attach(&default_camera_);
	camera_ = &default_camera_;
}

SceneManager::SceneManager(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain) : SceneManager()
{
	setDevice(device, context, swapChain);
}

void SceneManager::setDevice(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain)
{
	device_ = device;
	context_ = context;
	swap_chain_ = swapChain;
}


void SceneManager::setEnvironmentMap(const std::string& filename)
{
	CreateTextureFromFile(filename.c_str(), TextureLoadInfo{"SceneManager Environment Map"}, device(), &environment_map_);
    StateTransitionDesc barriers [] =
    {
        {environment_map_, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
    };
    context()->TransitionResourceStates(_countof(barriers), barriers);
}

RefCntAutoPtr<ITexture> SceneManager::getEnvironmentMap() const
{
	return environment_map_;
}

void SceneManager::render()
{
    clearRenderQueues();

	getRoot()->updateTransforms();

	render_matrices_.proj = camera_->getProjectionMatrix().cast<Real>();

	Matrix4 view_inv;
	view_inv = camera_->getNode()->getDerivedTransform();
	render_matrices_.view = view_inv.inverse();

	render_matrices_.view_proj = render_matrices_.proj*render_matrices_.view;
	//viewProjInv_ = viewProj_.inverse();
	render_matrices_.camera_world_position = view_inv.block<3,1>(0,3);

	collectRenderables(getRoot());

	last_pso_in_render_ = nullptr;
	last_material_in_render_ = nullptr;

	//std::cout << std::endl << "RENDER " << std::endl;
	for(auto& p : renderQueues_)
	{
	    RenderQueue& queue = p.second;
	    //std::cout << "RenderQueue: " << queue.size() << std::endl;
        for(Object* obj : queue)
        {
            if(obj->typeId() == type_id<Renderable>())
            {
                Renderable* r = reinterpret_cast<Renderable*>(obj);
                Matrix4 world = r->getNode()->getDerivedTransform();
                render_matrices_.world_view_proj = render_matrices_.view_proj * world;
                render_matrices_.world_view = render_matrices_.view * world;
                render(r, render_matrices_);
            }

            if(obj->typeId() == type_id<RawRenderable>())
            {
                RawRenderable* r = reinterpret_cast<RawRenderable*>(obj);
                Matrix4 world = r->getNode()->getDerivedTransform();
                render_matrices_.world_view_proj = render_matrices_.view_proj * world;
                render_matrices_.world_view = render_matrices_.view * world;
                render(r, render_matrices_);
            }
        }
	}
}

void SceneManager::collectRenderables(Node* node)
{
    if(!node->isEnabled())
        return;

	for(Object* obj : node->getObjects())
	{

	    {
            Renderable* r = obj->cast<Renderable>();
            if(r)
                renderQueues_[r->render_order].push_back(obj);
	    }
        {
            RawRenderable* r = obj->cast<RawRenderable>();
            if(r)
                renderQueues_[r->render_order_].push_back(obj);
        }

	}

	for(Node* child : node->getChildren())
		collectRenderables(child);
}

void SceneManager::clearRenderQueues()
{
    for(auto &p : renderQueues_)
        p.second.clear();
}


void SceneManager::render(Renderable* r, const Matrices& matrices)
{
	const Matrices* prevrender_matrices = current_render_matrices_;
	current_render_matrices_ = &matrices;

	if(r->pso_needs_update_)
	{
		PipelineStateDesc desc;
		desc.Name = "Renderable PSO";
		desc.IsComputePipeline = false;
		desc.GraphicsPipeline.NumRenderTargets  = 1;
		desc.GraphicsPipeline.RTVFormats[0]     = swapChain()->GetDesc().ColorBufferFormat;
		desc.GraphicsPipeline.DSVFormat         = swapChain()->GetDesc().DepthBufferFormat;
		desc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		desc.GraphicsPipeline.PrimitiveTopology = r->primitive_topology;
		desc.GraphicsPipeline.RasterizerDesc    = r->rasterizer_desc;
		desc.GraphicsPipeline.DepthStencilDesc  = r->depth_stencil_desc;

		desc.GraphicsPipeline.InputLayout.LayoutElements = r->input_layout.data();
		desc.GraphicsPipeline.InputLayout.NumElements = r->input_layout.size();

		r->material->setupPSODesc(desc);

		std::size_t pso_hash = hash_value(desc);
		IPipelineState* pso = psoManager_.getPSO(device(),desc);

		//std::cout << "r=" << r << std::endl;
		//std::cout << " order: " << r->_renderOrder << std::endl;

		// if pso has changed
		if(pso != r->pso_)
		{
			r->pso_ = pso;
			r->material->bindPSO(r->pso_);
			pso->CreateShaderResourceBinding(&r->srb_, true); // TODO: is it sufficient to have one srb per pso instead per renderable??
			r->material->bindSRB(r->srb_);
		}

		r->pso_needs_update_ = false;
	}

	{
		auto constants = r->material->shader_program_->mapConstant<dg::CommonConstantsVS>(context(), "CommonConstantsVS");
		matrix_to_float4x4t(current_render_matrices_->world_view_proj, constants->g_worldViewProj);
		matrix_to_float4x4t(current_render_matrices_->world_view, constants->g_worldView);
		matrix_to_float4x4t(current_render_matrices_->view, constants->g_view);
		/// TODO
		//context()->CommitShaderResources(r->_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	if(r->material.get()!=last_material_in_render_)
	{
		//std::cout << "prepareMaterial" << std::endl;
		r->material->prepareForRender(context());
		last_material_in_render_ = r->material.get();
	}

	// Bind vertex and index buffers
    Uint32   offset   = 0;
    IBuffer* buffs[] = {r->vertex_buffer};
    context()->SetVertexBuffers(0, 1, buffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    context()->SetIndexBuffer(r->index_buffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
	if(r->pso_ != last_pso_in_render_)
    {
    	//std::cout << "PSO changed" << std::endl;
    	context()->SetPipelineState(r->pso_);
    	last_pso_in_render_ = r->pso_;
    }
    context()->CommitShaderResources(r->srb_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawIndexedAttribs attr;     // This is an indexed draw call
    attr.IndexType  = VT_UINT32; // Index type
    attr.NumIndices = r->index_count;
    attr.Flags = DRAW_FLAG_VERIFY_ALL;
    context()->DrawIndexed(attr);

    current_render_matrices_ = prevrender_matrices;
}

void SceneManager::render(RawRenderable* r, const Matrices& matrices)
{
	const Matrices* prev_render_matrices = current_render_matrices_;
	current_render_matrices_ = &matrices;
	//_worldViewProj = _viewProj * world;
	//_worldViewProjInv = _worldViewProj.inverse();

	r->render(this);
	current_render_matrices_ = prev_render_matrices;
}

}
