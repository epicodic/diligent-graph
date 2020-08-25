#include <dg/platform/render_window.hpp>

#include "render_window_private.hpp"

#include <map>
#include <iostream>

#include <dlfcn.h>
#include <libgen.h>    // dirname

#include <imgui/imgui.h>


namespace dg {

RenderWindow::RenderWindow(RenderWindowListener* listener) : d(new RenderWindowPrivate), _listener(listener)
{
}

RenderWindow::~RenderWindow()
{

}

void RenderWindow::setListener(RenderWindowListener* listener)
{
    _listener = listener;
}


IRenderDevice* RenderWindow::device()
{
    return d->device;
}

IDeviceContext* RenderWindow::context()
{
    return d->context;
}

ISwapChain* RenderWindow::swapChain()
{
    return d->swapChain;
}

IEngineFactory* RenderWindow::engineFactory()
{
    return d->engineFactory;
}



void RenderWindow::spin()
{
    while(spinOnce());
}



void RenderWindow::addShortcut(const KeySequence &key_seq,
        std::function<void()> on_activate, const std::string &info_text)
{
    _shortcuts.addShortcut(key_seq, on_activate, info_text);
}

void RenderWindow::addShortcut(const std::string &key_seq,
        std::function<void()> on_activate, const std::string &info_text)
{
    _shortcuts.addShortcut(key_seq, on_activate, info_text);
}

ImFont* RenderWindow::addFont(const std::string& font_filename, int font_size)
{
    ImGuiIO& io  = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(font_filename.c_str(), font_size);
    if(font!=nullptr)
        d->gui->UpdateFontsTexture();

    return font;
}

ImFont* RenderWindow::addFontCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, int font_size)
{
    ImGuiIO& io  = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(compressed_ttf_data, compressed_ttf_size, font_size);
    if(font!=nullptr)
        d->gui->UpdateFontsTexture();
    return font;
}

void RenderWindow::setDefaultFont(ImFont* font)
{
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = font;
}

// EVENTS

void RenderWindow::initialize()
{
    const SwapChainDesc& scdesc = swapChain()->GetDesc();

    d->gui.reset(new ImGuiIntegration(device(), scdesc.ColorBufferFormat, scdesc.DepthBufferFormat, scdesc.Width, scdesc.Height));

    _listener->initialize();
}

void RenderWindow::render()
{
    d->gui->NewFrame();

    _listener->render();

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    Cursor cursor = Cursor::Arrow;

    switch(imgui_cursor)
    {
        case ImGuiMouseCursor_Arrow: cursor = Cursor::Arrow; break;
        case ImGuiMouseCursor_TextInput: cursor = Cursor::IBeam; break;
        case ImGuiMouseCursor_ResizeAll: cursor = Cursor::SizeAll; break;
        case ImGuiMouseCursor_ResizeNS: cursor = Cursor::SizeVer; break;
        case ImGuiMouseCursor_ResizeEW: cursor = Cursor::SizeHor; break;
        case ImGuiMouseCursor_ResizeNESW: cursor = Cursor::SizeBDiag; break;
        case ImGuiMouseCursor_ResizeNWSE: cursor = Cursor::SizeFDiag; break;
        case ImGuiMouseCursor_Hand: cursor = Cursor::Arrow; break;
        case ImGuiMouseCursor_NotAllowed: cursor = Cursor::Forbidden; break;
    }

    setCursor(cursor);

    d->gui->Render(context());

    if(!_listener->present())
        swapChain()->Present();
}

void RenderWindow::resizeEvent(const ResizeEvent& event)
{
    if(d->gui)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(event.width, event.height);
    }

    _listener->resizeEvent(event);


}

void RenderWindow::focusOutEvent()
{
    _shortcuts.focusOutEvent();
    _listener->focusOutEvent();
}

void RenderWindow::keyPressEvent(const KeyEvent& event)
{
    _shortcuts.keyPressEvent(event);
    d->gui->keyPressEvent(event);
    _listener->keyPressEvent(event);
}

void RenderWindow::keyReleaseEvent(const KeyEvent& event)
{
    _shortcuts.keyReleaseEvent(event);
    d->gui->keyReleaseEvent(event);
    _listener->keyReleaseEvent(event);
}

void RenderWindow::mouseMoveEvent(const MouseEvent& event)
{
    d->gui->mouseMoveEvent(event);
    _listener->mouseMoveEvent(event);
}

void RenderWindow::mousePressEvent(const MouseEvent& event)
{
    d->gui->mousePressEvent(event);
    _listener->mousePressEvent(event);
}

void RenderWindow::mouseReleaseEvent(const MouseEvent& event)
{
    d->gui->mouseReleaseEvent(event);
    _listener->mouseReleaseEvent(event);
}

void RenderWindow::wheelEvent(const WheelEvent& event)
{
    d->gui->wheelEvent(event);
    _listener->wheelEvent(event);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef std::map<std::string, RenderWindowFactory*> FactoryRegistryMap;

FactoryRegistryMap& factoryRegistry()
{
    static FactoryRegistryMap s_factoryRegistry;
    return s_factoryRegistry;
}

std::string diligent_getLibraryPath(const std::string& backendName)
{
    // obtain library information containing this method
    Dl_info info;
    int res = dladdr((void*)&diligent_getLibraryPath, &info);

    if (!res)
        return backendName;

    // get dir from library name
    char* libname = strdup(info.dli_fname);
    char* path = dirname(libname);
    std::string spath(path);
    free(libname);

    return spath+"/libdiligent-graph-"+backendName+".so";
}

void loadLibrary(const std::string& filename)
{
    dlerror();    // clear any existing error
    void* handle = dlopen(filename.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr)
        std::cerr << "Failed to load library '" << filename << "' :" << dlerror() << std::endl;
}


RenderWindowFactory* RenderWindowFactory::getFactory(const std::string& renderBackend)
{
    loadLibrary(diligent_getLibraryPath(renderBackend));

    FactoryRegistryMap& registry = factoryRegistry();
    auto it = registry.find(renderBackend);
    if(it==registry.end())
        return nullptr;
    return it->second;
}

void RenderWindowFactory::registerFactory(const std::string& renderBackend, RenderWindowFactory* factory)
{
    std::cout << "Registering RenderWindowFactory " << renderBackend << ", " << factory << std::endl;
    FactoryRegistryMap& registry = factoryRegistry();
    registry[renderBackend] = factory;
}

void RenderWindowFactory::unregisterFactory(RenderWindowFactory* factory)
{
    std::cout << "Unregistering RenderWindowFactory " << factory << std::endl;
    FactoryRegistryMap& registry = factoryRegistry();
    for(auto& p : registry)
        if(p.second == factory)
            p.second = nullptr;
}



}
