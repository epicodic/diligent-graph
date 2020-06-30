#include <dg/objects/canvas_manual_layer.hpp>

namespace dg {

CanvasManualLayer::CanvasManualLayer(CanvasObject* parent_object) : CanvasLayer(parent_object)
{
	_manualObject = dg::ManualObject::make_unique(getParentObject()->getSceneManager());
}

void CanvasManualLayer::render()
{
	if(!_manualObject)
		return;

	for(const auto& p : _manualObject->getSections())
	{
		getParentObject()->getSceneManager()->render(p.get(), getParentObject()->getRenderMatrices());
	}
}

void CanvasManualLayer::setOpacity(float opacity)
{
	// TODO:
	std::cout << "TODO: IMPLEMENT ME: " << __PRETTY_FUNCTION__ << std::endl;
}

}
