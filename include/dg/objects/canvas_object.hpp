#pragma once

#include <memory>

#include <dg/scene/raw_renderable.hpp>

// fwd decl:
class ImGuiContext;
class ImDrawListSharedData;
class ImDrawList;


namespace dg {

class ManualObject;

class CanvasObject;

class CanvasLayer
{
public:

	CanvasLayer(CanvasObject* parent_object);
	virtual ~CanvasLayer() = default;

public:

	CanvasObject* getParentObject() { return _parent_object; }
	const CanvasObject* getParentObject() const { return _parent_object; }

public:

	virtual bool isBackground() const { return false; }

	virtual void render() = 0;
	virtual void setOpacity(float opacity) {}

	virtual Eigen::Vector2f getSize() const { return Eigen::Vector2f(0.0f,0.0f); }

protected:
	friend class CanvasObject;

	virtual void onAttached(Node* node) {}
	virtual void onDetached(Node* node) {}

private:

	CanvasObject* _parent_object = nullptr;

};

class CanvasDrawingLayer;
class CanvasImageLayer;
class CanvasManualLayer;

class CanvasObject : public RawRenderable
{
public:
	DG_PTR(CanvasObject)

public:

	CanvasObject(SceneManager* manager);
	virtual ~CanvasObject();


public:

	void setOverlayMode(bool overlay_mode);
	bool isOverlayMode() const;
	void setOverlayWidth(float width);
	void setOverlayPosition(float x, float y);
	void setOverlayAlignment(bool align_right, bool align_bottom);

public:

	virtual void render(SceneManager* manager) override;

public:

	CanvasDrawingLayer* createDrawingLayer();
	CanvasImageLayer* createImageLayer();
	CanvasManualLayer* createManualLayer();

	void addLayer(std::unique_ptr<CanvasLayer> layer);

public:

	SceneManager* getSceneManager();

	ImDrawListSharedData* getDrawListSharedData();
	ImGuiContext* getImGuiContext();
	ImDrawList* createDrawList();

	void renderDrawList(ImDrawList* draw_list, float opacity = 0.0f);

	const SceneManager::Matrices& getRenderMatrices();

	unsigned int getStencilId();

protected:

	virtual void onAttached(Node* node) override;
	virtual void onDetached(Node* node) override;

private:

	struct Pimpl;
	std::unique_ptr<Pimpl> d;
};


}
