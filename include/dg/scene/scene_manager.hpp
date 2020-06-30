#pragma once

#include <deque>

#include "node.hpp"

#include <dg/core/fwds.hpp>

#include <dg/scene/camera.hpp>
#include <dg/scene/node.hpp>
#include <dg/scene/render_order.hpp>

#include <dg/internal/pso_manager.hpp>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace dg {

class Renderable;
class RawRenderable;
class IMaterial;


class SceneManager
{
public:

	SceneManager();
	SceneManager(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain);

	void setDevice(IRenderDevice* device, IDeviceContext* context, ISwapChain* swapChain);

public:

	IRenderDevice* device()	{ return _device; }

	IDeviceContext* context() { return _context; }

	ISwapChain* swapChain() { return _swapChain; }


public:

	Node* getRoot() { return _root.get(); }
	const Node* getRoot() const { return _root.get(); }

	Camera* getCamera() { return _camera; }
	const Camera* getCamera() const { return _camera; }

	void setCamera(Camera* camera) { _camera = camera; }

public:

	void render();


	struct Matrices
	{
		Matrix4 worldViewProj;
		Matrix4 worldView;

		Matrix4 viewProj;
		Matrix4 view;
		Matrix4 proj;
		Vector3 cameraWorldPosition;
	};


	void render(Renderable* r, const Matrices& matrices);


	void render(RawRenderable* r, const Matrices& matrices);


	// the following values are updated and valid during render() (i.e. in the render() methods of the renderables)
	const Matrix4& getWorldViewProj() const { return _current_render_matrices->worldViewProj; }
	//const Matrix4& getWorldViewProjInv() const { return _worldViewProjInv; }
	const Matrix4& getWorldView() const { return _current_render_matrices->worldView; }
	//const Matrix4& getWorldViewInv() const { return _worldViewInv; }
	const Matrix4& getView() const { return _current_render_matrices->view; }
	//const Matrix4& getViewInv() const { return _viewInv; }
	const Matrix4& getViewProj() const { return _current_render_matrices->viewProj; }
	//const Matrix4& getViewProjInv() const { return _viewProjInv; }
	const Matrix4& getProj() const { return _current_render_matrices->proj; }
	const Vector3& getCameraWorldPosition() const { return _current_render_matrices->cameraWorldPosition; }

	const SceneManager::Matrices& getRenderMatrices() { return *_current_render_matrices; }


public:

	unsigned int requestStencilId() { return _next_free_stencil_id++; }

public:

	void setEnvironmentMap(const std::string& filename);
	RefCntAutoPtr<ITexture> getEnvironmentMap() const;

private:

	void collectRenderables(Node* node);

	void clearRenderQueues();


private:

	dg::PSOManager _psoManager;

	IRenderDevice*  _device = nullptr;
	IDeviceContext* _context = nullptr;
	ISwapChain*     _swapChain = nullptr;

	//std::deque<Object*> _renderQueue;
	typedef std::deque<Object*> RenderQueue;
	std::map<RenderOrder, RenderQueue> _renderQueues;

	Node::Ptr _root;
	Camera* _camera = nullptr;

	Camera _default_camera;
	Node::Ptr _default_camera_node;

	Matrices _render_matrices;
	const Matrices* _current_render_matrices = nullptr;

	IPipelineState* _lastPSOInRender = nullptr;
	IMaterial* _lastMaterialInRender = nullptr;
	bool _needCommonConstantsVSUpdateInRender = true;

	unsigned int _next_free_stencil_id=10;

	RefCntAutoPtr<ITexture> _environment_map;


};


}
