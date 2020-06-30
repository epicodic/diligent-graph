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
	_root = Node::make();
	_default_camera_node = getRoot()->createChild();
	_default_camera_node->attach(&_default_camera);
	_camera = &_default_camera;
}

SceneManager::SceneManager(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain) : SceneManager()
{
	setDevice(device, context, swapChain);
}

void SceneManager::setDevice(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain)
{
	_device = device;
	_context = context;
	_swapChain = swapChain;
}


void SceneManager::setEnvironmentMap(const std::string& filename)
{
	CreateTextureFromFile(filename.c_str(), TextureLoadInfo{"SceneManager Environment Map"}, device(), &_environment_map);
    StateTransitionDesc barriers [] =
    {
        {_environment_map, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
    };
    context()->TransitionResourceStates(_countof(barriers), barriers);
}

RefCntAutoPtr<ITexture> SceneManager::getEnvironmentMap() const
{
	return _environment_map;
}

void SceneManager::render()
{
    clearRenderQueues();

	getRoot()->updateTransforms();

	_render_matrices.proj = _camera->getProjectionMatrix().cast<Real>();

	Matrix4 _viewInv;
	_viewInv = _camera->getNode()->getDerivedTransform();
	_render_matrices.view = _viewInv.inverse();

	_render_matrices.viewProj = _render_matrices.proj*_render_matrices.view;
	//_viewProjInv = _viewProj.inverse();
	_render_matrices.cameraWorldPosition = _viewInv.block<3,1>(0,3);

	collectRenderables(getRoot());

	_lastPSOInRender = nullptr;
	_lastMaterialInRender = nullptr;

	//std::cout << std::endl << "RENDER " << std::endl;
	for(auto& p : _renderQueues)
	{
	    RenderQueue& queue = p.second;
	    //std::cout << "RenderQueue: " << queue.size() << std::endl;
        for(Object* obj : queue)
        {
            if(obj->typeId() == type_id<Renderable>())
            {
                Renderable* r = reinterpret_cast<Renderable*>(obj);
                Matrix4 world = r->getNode()->getDerivedTransform();
                _render_matrices.worldViewProj = _render_matrices.viewProj * world;
                _render_matrices.worldView = _render_matrices.view * world;
                render(r, _render_matrices);
            }

            if(obj->typeId() == type_id<RawRenderable>())
            {
                RawRenderable* r = reinterpret_cast<RawRenderable*>(obj);
                Matrix4 world = r->getNode()->getDerivedTransform();
                _render_matrices.worldViewProj = _render_matrices.viewProj * world;
                _render_matrices.worldView = _render_matrices.view * world;
                render(r, _render_matrices);
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
                _renderQueues[r->_render_order].push_back(obj);
	    }
        {
            RawRenderable* r = obj->cast<RawRenderable>();
            if(r)
                _renderQueues[r->_render_order].push_back(obj);
        }

	}

	for(Node* child : node->getChildren())
		collectRenderables(child);
}

void SceneManager::clearRenderQueues()
{
    for(auto &p : _renderQueues)
        p.second.clear();
}


void SceneManager::render(Renderable* r, const Matrices& matrices)
{
	const Matrices* prev_render_matrices = _current_render_matrices;
	_current_render_matrices = &matrices;

	if(r->_pso_needs_update)
	{
		PipelineStateDesc desc;
		desc.Name = "Renderable PSO";
		desc.IsComputePipeline = false;
		desc.GraphicsPipeline.NumRenderTargets  = 1;
		desc.GraphicsPipeline.RTVFormats[0]     = swapChain()->GetDesc().ColorBufferFormat;
		desc.GraphicsPipeline.DSVFormat         = swapChain()->GetDesc().DepthBufferFormat;
		desc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		desc.GraphicsPipeline.PrimitiveTopology = r->_primitiveTopology;
		desc.GraphicsPipeline.RasterizerDesc    = r->_rasterizerDesc;
		desc.GraphicsPipeline.DepthStencilDesc  = r->_depthStencilDesc;

		desc.GraphicsPipeline.InputLayout.LayoutElements = r->_inputLayout.data();
		desc.GraphicsPipeline.InputLayout.NumElements = r->_inputLayout.size();

		r->_material->setupPSODesc(desc);

		std::size_t pso_hash = hash_value(desc);
		IPipelineState* pso = _psoManager.getPSO(device(),desc);

		//std::cout << "r=" << r << std::endl;
		//std::cout << " order: " << r->_renderOrder << std::endl;

		// if pso has changed
		if(pso != r->_pso)
		{
			r->_pso = pso;
			r->_material->bindPSO(r->_pso);
			pso->CreateShaderResourceBinding(&r->_srb, true); // TODO: is it sufficient to have one srb per pso instead per renderable??
			r->_material->bindSRB(r->_srb);
		}

		r->_pso_needs_update = false;
	}

	{
		auto constants = r->_material->_shader_program->mapConstant<dg::CommonConstantsVS>(context(), "CommonConstantsVS");
		matrix_to_float4x4t(_current_render_matrices->worldViewProj, constants->g_worldViewProj);
		matrix_to_float4x4t(_current_render_matrices->worldView, constants->g_worldView);
		matrix_to_float4x4t(_current_render_matrices->view, constants->g_view);
		/// TODO
		//context()->CommitShaderResources(r->_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	if(r->_material.get()!=_lastMaterialInRender)
	{
		//std::cout << "prepareMaterial" << std::endl;
		r->_material->prepareForRender(context());
		_lastMaterialInRender = r->_material.get();
	}

	// Bind vertex and index buffers
    Uint32   offset   = 0;
    IBuffer* pBuffs[] = {r->_vertexBuffer};
    context()->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    context()->SetIndexBuffer(r->_indexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
	if(r->_pso != _lastPSOInRender)
    {
    	//std::cout << "PSO changed" << std::endl;
    	context()->SetPipelineState(r->_pso);
    	_lastPSOInRender = r->_pso;
    }
    context()->CommitShaderResources(r->_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawIndexedAttribs attr;     // This is an indexed draw call
    attr.IndexType  = VT_UINT32; // Index type
    attr.NumIndices = r->_indexCount;
    attr.Flags = DRAW_FLAG_VERIFY_ALL;
    context()->DrawIndexed(attr);

    _current_render_matrices = prev_render_matrices;
}

void SceneManager::render(RawRenderable* r, const Matrices& matrices)
{
	const Matrices* prev_render_matrices = _current_render_matrices;
	_current_render_matrices = &matrices;
	//_worldViewProj = _viewProj * world;
	//_worldViewProjInv = _worldViewProj.inverse();

	r->render(this);
	_current_render_matrices = prev_render_matrices;
}

}
