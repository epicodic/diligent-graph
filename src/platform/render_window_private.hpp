#pragma once

#include <memory>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include <dg/gui/imgui_integration.hpp>

#include <dg/platform/cursor.hpp>

namespace dg {

using namespace Diligent;

class RenderWindowPrivate
{
public:
	RefCntAutoPtr<IRenderDevice>  device;
	RefCntAutoPtr<IDeviceContext> context;
	RefCntAutoPtr<ISwapChain>     swapChain;
	IEngineFactory*               engineFactory;

	std::unique_ptr<ImGuiIntegration> gui;
};

}
