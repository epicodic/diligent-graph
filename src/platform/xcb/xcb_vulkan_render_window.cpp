
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


    xcb_map_window(connection_, window_);


    Diligent::EngineVkCreateInfo eng_vk_attribs;
    eng_vk_attribs.EnableValidation = true;

    auto* factory_vk = Diligent::GetEngineFactoryVk();
    factory_vk->CreateDeviceAndContextsVk(eng_vk_attribs, &d->device, &d->context);
    dg::SwapChainDesc     sc_desc;
    dg::LinuxNativeWindow xcb_window;
    xcb_window.WindowId       = window_;
    xcb_window.pXCBConnection = connection_;
    factory_vk->CreateSwapChainVk(d->device, d->context, sc_desc, xcb_window, &d->swap_chain);

    xcb_flush(connection);

    initialize();
    resizeEvent(ResizeEvent{width_,  height_});
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

XCBVulkanRenderWindowFactory g_xcb_vulkan_render_window_factory;


}
