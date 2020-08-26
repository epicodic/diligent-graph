
#include "xcb_render_window.hpp"
#include "../render_window_private.hpp"

#include <dg/core/common.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>

#include <memory>


namespace dg {

class XCBVulkanRenderWindow : public XCBRenderWindow
{

public:

	XCBVulkanRenderWindow(RenderWindowListener* listener);

	virtual void create(const CreationOptions& options) override;
};


XCBVulkanRenderWindow::XCBVulkanRenderWindow(RenderWindowListener* listener) : XCBRenderWindow(listener)
{
}

void XCBVulkanRenderWindow::create(const CreationOptions& options)
{
	int default_screen;

    int scr         = 0;
    xcb_connection_t* connection = xcb_connect(nullptr, &scr);
    if (connection == nullptr || xcb_connection_has_error(connection))
        DG_THROW("Unable to make an XCB connection");

    XCBKeyboard::initializeXKB(connection);

	const xcb_setup_t*    setup = xcb_get_setup(connection);
	xcb_screen_iterator_t iter  = xcb_setup_roots_iterator(setup);
	while (scr-- > 0)
		xcb_screen_next(&iter);

	auto screen = iter.data;

	createWindow(connection, screen->root, screen->root_visual, options);


    xcb_map_window(_connection, _window);


    Diligent::EngineVkCreateInfo EngVkAttribs;
    EngVkAttribs.EnableValidation = true;
/*
    EngVkAttribs.MainDescriptorPoolSize.MaxDescriptorSets /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumSeparateSamplerDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumCombinedSamplerDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumSampledImageDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumStorageImageDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumUniformBufferDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumUniformTexelBufferDescriptors /= 8;
    EngVkAttribs.MainDescriptorPoolSize.NumStorageTexelBufferDescriptors /= 8;
*/
    auto* pFactoryVk = Diligent::GetEngineFactoryVk();
    pFactoryVk->CreateDeviceAndContextsVk(EngVkAttribs, &d->device, &d->context);
    dg::SwapChainDesc     SCDesc;
    dg::LinuxNativeWindow XCBWindow;
    XCBWindow.WindowId       = _window;
    XCBWindow.pXCBConnection = _connection;
    pFactoryVk->CreateSwapChainVk(d->device, d->context, SCDesc, XCBWindow, &d->swapChain);

    xcb_flush(connection);

    initialize();
    resizeEvent(ResizeEvent{_width,  _height});
}




class XCBVulkanRenderWindowFactory : public RenderWindowFactory
{
public:

	XCBVulkanRenderWindowFactory()
	{
		registerFactory("vulkan", this);
	}

	~XCBVulkanRenderWindowFactory()
	{
		unregisterFactory(this);
	}

	virtual RenderWindow* createRenderWindow(RenderWindowListener* listener) override
	{
		RenderWindow* win = new XCBVulkanRenderWindow(listener);
		return win;
	}

};

XCBVulkanRenderWindowFactory g_XCBVulkanRenderWindowFactory;


}
