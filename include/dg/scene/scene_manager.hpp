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

	IRenderDevice* device()	{ return device_; }

	IDeviceContext* context() { return context_; }

	ISwapChain* swapChain() { return swap_chain_; }


public:

	Node* getRoot() { return root_.get(); }
	const Node* getRoot() const { return root_.get(); }

	Camera* getCamera() { return camera_; }
	const Camera* getCamera() const { return camera_; }

	void setCamera(Camera* camera) { camera_ = camera; }

public:

	void render();


	struct Matrices
	{
		Matrix4 world_view_proj;
		Matrix4 world_view;

		Matrix4 view_proj;
		Matrix4 view;
		Matrix4 proj;
		Vector3 camera_world_position;
	};


	void render(Renderable* r, const Matrices& matrices);


	void render(RawRenderable* r, const Matrices& matrices);


	// the following values are updated and valid during render() (i.e. in the render() methods of the renderables)
	const Matrix4& getWorldViewProj() const { return current_render_matrices_->world_view_proj; }
	const Matrix4& getWorldView() const { return current_render_matrices_->world_view; }
	const Matrix4& getView() const { return current_render_matrices_->view; }
	const Matrix4& getViewProj() const { return current_render_matrices_->view_proj; }
	const Matrix4& getProj() const { return current_render_matrices_->proj; }
	const Vector3& getCameraWorldPosition() const { return current_render_matrices_->camera_world_position; }

	const SceneManager::Matrices& getRenderMatrices() { return *current_render_matrices_; }

public:

	unsigned int requestStencilId() { return next_free_stencil_id_++; }

public:

	void setEnvironmentMap(const std::string& filename);
	RefCntAutoPtr<ITexture> getEnvironmentMap() const;

private:

	void collectRenderables(Node* node);

	void clearRenderQueues();


private:

	dg::PSOManager psoManager_;

	IRenderDevice*  device_ = nullptr;
	IDeviceContext* context_ = nullptr;
	ISwapChain*     swap_chain_ = nullptr;

	using RenderQueue = std::deque<Object *>;
	std::map<RenderOrder, RenderQueue> renderQueues_;

	Node::Ptr root_;
	Camera* camera_ = nullptr;

	Camera default_camera_;
	Node::Ptr default_camera_node_;

	Matrices render_matrices_;
	const Matrices* current_render_matrices_ = nullptr;

	IPipelineState* last_pso_in_render_ = nullptr;
	IMaterial* last_material_in_render_ = nullptr;
	bool need_common_constants_vs_update_in_render_ = true;

	unsigned int next_free_stencil_id_=10;

	RefCntAutoPtr<ITexture> environment_map_;


};


}
