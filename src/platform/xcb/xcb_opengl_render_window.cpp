
#include "xcb_render_window.hpp"
#include "../render_window_private.hpp"

#include <dg/core/common.hpp>

#include <memory>

#include <GL/glx.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

//extern "C" {
#include <X11/Xlib-xcb.h> /* for XGetXCBConnection, link with libX11-xcb */
//}

#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>

#ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
#    define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif

#ifndef GLX_CONTEXT_MINOR_VERSION_ARB
#    define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif

#ifndef GLX_CONTEXT_FLAGS_ARB
#    define GLX_CONTEXT_FLAGS_ARB 0x2094
#endif

#ifndef GLX_CONTEXT_DEBUG_BIT_ARB
#    define GLX_CONTEXT_DEBUG_BIT_ARB 0x0001
#endif

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, int, const int*);


namespace dg {

class XCBOpenGLRenderWindow : public XCBRenderWindow
{

public:

	XCBOpenGLRenderWindow(RenderWindowListener* listener);

	virtual void create() override;

private:

    struct X11DisplayDeleter {
        void operator()(Display* display) const { XCloseDisplay(display); }
    };
    using X11DisplayPtr = std::unique_ptr<Display, X11DisplayDeleter>;

    X11DisplayPtr _display;

    std::vector<char> _grab_buffer;
};



XCBOpenGLRenderWindow::XCBOpenGLRenderWindow(RenderWindowListener* listener) : XCBRenderWindow(listener)
{
}

void XCBOpenGLRenderWindow::create()
{
	int default_screen;

	// Open Xlib Display
	Display* display = XOpenDisplay(0);
	_display.reset(display);


	if(!_display)
		DG_THROW("Can't open display");

	default_screen = DefaultScreen(display);

	// Get the XCB connection from the display
	xcb_connection_t *connection =
			XGetXCBConnection(display);

	if(!connection)
		DG_THROW("Can't get xcb connection from display");

	// Acquire event queue ownership
	XSetEventQueueOwner(display, XCBOwnsEventQueue);

	// Find XCB screen //
	xcb_screen_t *screen = 0;
	xcb_screen_iterator_t screen_iter =
			xcb_setup_roots_iterator(xcb_get_setup(connection));
	for(int screen_num = default_screen;
			screen_iter.rem && screen_num > 0;
			--screen_num, xcb_screen_next(&screen_iter));
	screen = screen_iter.data;


	int visualID = 0;

	// Query framebuffer configurations that match visual_attribs

	static int visual_attribs[] =
	{
	    //GLX_X_RENDERABLE, True,
	    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
	    GLX_RENDER_TYPE, GLX_RGBA_BIT,
	    //GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
	    GLX_RED_SIZE, 8,
	    GLX_GREEN_SIZE, 8,
	    GLX_BLUE_SIZE, 8,
	    GLX_ALPHA_SIZE, 8,
	    GLX_DEPTH_SIZE, 24,
	    GLX_STENCIL_SIZE, 8,
	    GLX_DOUBLEBUFFER, True,
	    //GLX_SAMPLE_BUFFERS  , 1,
	    GLX_SAMPLES         , 4,
	    None
	};

	GLXFBConfig *fb_configs = 0;
	int num_fb_configs = 0;
	fb_configs = glXChooseFBConfig(display, default_screen, visual_attribs, &num_fb_configs);
	if(!fb_configs || num_fb_configs == 0)
		DG_THROW("glXGetFBConfigs failed\n");

	std::cout << "Found matching FB configs: " << num_fb_configs << std::endl;

	// Select first framebuffer config and query visualID
	GLXFBConfig fb_config = fb_configs[0];
	glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);


	/* Create XID's for colormap and window */
	xcb_colormap_t colormap = xcb_generate_id(connection);

	/* Create colormap */
	xcb_create_colormap(
			connection,
			XCB_COLORMAP_ALLOC_NONE,
			colormap,
			screen->root,
			visualID
	);

	createWindow(connection, screen->root, visualID, 1024, 768, colormap);


    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fb_config);


    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
    {
        // Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
        GLXContext ctx_old         = glXCreateContext(display, vi, 0, GL_TRUE);
        //GLXContext ctx_old         = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, ctx_old);
    }

    if (glXCreateContextAttribsARB == nullptr)
        DG_THROW("glXCreateContextAttribsARB entry point not found.");

    int flags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
//    flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

    int major_version = 4;
    int minor_version = 3;

    static int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, major_version,
        GLX_CONTEXT_MINOR_VERSION_ARB, minor_version,
        GLX_CONTEXT_FLAGS_ARB, flags,
        None
    };

    GLXContext    ctx  = glXCreateContextAttribsARB(display, fb_config, NULL, 1, context_attribs);
    if (!ctx)
    	DG_THROW("Failed to create GL context.\n");

    if(!glXMakeCurrent(display, _window, ctx))
    	DG_THROW("glxMakeCurrent failed!");



    Diligent::SwapChainDesc SCDesc;
    SCDesc.Usage |= SWAP_CHAIN_USAGE_COPY_SOURCE;

    Diligent::Uint32        NumDeferredCtx = 0;
    // Declare function pointer
    auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();

    Diligent::EngineGLCreateInfo CreationAttribs;
    CreationAttribs.Window.WindowId = _window;
    CreationAttribs.Window.pDisplay = _display.get();
    pFactoryOpenGL->CreateDeviceAndSwapChainGL(
        CreationAttribs, &d->device, &d->context, SCDesc, &d->swapChain);

    xcb_flush(connection);

    initialize();
    resizeEvent(ResizeEvent{_width,  _height});
}

class XCBOpenGLRenderWindowFactory : public RenderWindowFactory
{
public:

	XCBOpenGLRenderWindowFactory()
	{
		registerFactory("opengl", this);
	}

	~XCBOpenGLRenderWindowFactory()
	{
		unregisterFactory(this);
	}

	virtual RenderWindow* createRenderWindow(RenderWindowListener* listener) override
	{
		RenderWindow* win = new XCBOpenGLRenderWindow(listener);
		return win;
	}

};

XCBOpenGLRenderWindowFactory g_XCBOpenGLRenderWindowFactory;


}
