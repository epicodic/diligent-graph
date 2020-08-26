
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

    virtual void create(const CreationOptions& options) override;

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


static bool create_context_error_handler_flag;

static int create_context_error_handler(Display *dpy, XErrorEvent *error)
{
    (void) dpy;
    (void) error->error_code;
    create_context_error_handler_flag = true;
    return 0;
}

void XCBOpenGLRenderWindow::create(const CreationOptions& options)
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
        //GLX_SAMPLES         , 4,
        None
    };

    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    fb_configs = glXChooseFBConfig(display, default_screen, visual_attribs, &num_fb_configs);
    if(!fb_configs || num_fb_configs == 0)
        DG_THROW("glXGetFBConfigs failed\n");

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

    createWindow(connection, screen->root, visualID, options, colormap);

    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fb_config);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
    {
        // Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
        GLXContext ctx_old         = glXCreateContext(display, vi, 0, GL_TRUE);
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, ctx_old);
    }

    if (glXCreateContextAttribsARB == nullptr)
        DG_THROW("glXCreateContextAttribsARB entry point not found.");

    int flags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
//    flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

    static const struct { int major, minor; } gl_versions[] = {
        {4, 6},
        {4, 5},
        {4, 4},
        {4, 3},
        {4, 2},
        {4, 1},
        {4, 0},

        {3, 3},
        {3, 2},
        {3, 1},
        {3, 0},
        // dont even try older versions

        {0, 0} // end of list
    };
    
    //// create the context with the latest possible core version
    GLXContext ctx = nullptr;

    XErrorHandler old_handler = XSetErrorHandler(create_context_error_handler); // set an error handler that eats all errors
    create_context_error_handler_flag = False;
    
    // probe through the OpenGL versions (latest first) and try to create the context 
    for (int i = 0; gl_versions[i].major > 0; i++) 
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, gl_versions[i].major,
            GLX_CONTEXT_MINOR_VERSION_ARB, gl_versions[i].minor,
            GLX_CONTEXT_FLAGS_ARB, flags,
            None
        };

        create_context_error_handler_flag = false;
        ctx  = glXCreateContextAttribsARB(display, fb_config, NULL, 1, context_attribs);
        if (create_context_error_handler_flag)
            ctx = nullptr;

        if(ctx)
        {
            std::cout << "Succeeded to create an OpenGL context with core version: " << gl_versions[i].major << "." << gl_versions[i].minor << std::endl;
            break;
        }
    }

    // restore error handler 
    XSetErrorHandler(old_handler);

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
