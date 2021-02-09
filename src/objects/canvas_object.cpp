
#include <dg/objects/canvas_object.hpp>
#include <dg/objects/canvas_drawing_layer.hpp>
#include <dg/objects/canvas_image_layer.hpp>
#include <dg/objects/canvas_manual_layer.hpp>

#include <dg/core/conversion.hpp>



#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


#include <dg/gui/imgui_impl_dg.hpp>

#include <dg/material/dynamic_texture.hpp>

#include <dg/objects/manual_object.hpp>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace dg {

using namespace Diligent;

struct CanvasObject::Pimpl
{
    std::unique_ptr<ImGuiImplDg> imgui_impl;
    ImDrawListSharedData* imgui_shared_data = nullptr;

    SceneManager* manager = nullptr;

    std::vector<std::unique_ptr<CanvasLayer> > layers;

    SceneManager::Matrices matrices;
    unsigned int stencil_id = 0;

    bool overlay_mode = false;
    float overlay_width = 1.0f;
    float overlay_x = 0.0f;
    float overlay_y = 0.0f;
    bool overlay_align_right = false;
    bool overlay_align_bottom = false;
    //float _height;
};

ImDrawList* g_draw_list = nullptr; //( TODO

CanvasObject::CanvasObject(SceneManager* manager)
{
    d.reset(new Pimpl);
    d->manager = manager;

    d->stencil_id = manager->requestStencilId();

    const SwapChainDesc& scdesc = manager->swapChain()->GetDesc();
    d->imgui_impl.reset(new ImGuiImplDg(manager->device(), scdesc.ColorBufferFormat, scdesc.DepthBufferFormat));

    ImGuiContext* oldctx = ImGui::GetCurrentContext();


    ImFont* font = ImGui::GetDefaultFont();

    ImGuiContext* ctx = getImGuiContext();
    ImGui::SetCurrentContext(ctx);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1.0f, 1.0f);
    io.FontDefault = font;

    ImGui::NewFrame();

    ImGui::SetCurrentFont(ImGui::GetDefaultFont());

    d->imgui_shared_data = ImGui::GetDrawListSharedData();

    ImGui::SetCurrentContext(oldctx);

}

CanvasObject::~CanvasObject()
{

}

void CanvasObject::setOverlayMode(bool overlay_mode)
{
    d->overlay_mode = overlay_mode;
}

bool CanvasObject::isOverlayMode() const
{
    return d->overlay_mode;
}

void CanvasObject::setOverlayWidth(float width)
{
    d->overlay_width = width;
}

void CanvasObject::setOverlayPosition(float x, float y)
{
    d->overlay_x = x;
    d->overlay_y = y;
}

void CanvasObject::setOverlayAlignment(bool align_right, bool align_bottom)
{
    d->overlay_align_right = align_right;
    d->overlay_align_bottom = align_bottom;
}

void CanvasObject::render(SceneManager* manager)
{
    d->matrices = manager->getRenderMatrices();

    if(d->overlay_mode)
    {
        // get the overall size in px
        Eigen::Vector2f size(1.0,1.0);
        for(const auto& p : d->layers)
            size = size.cwiseMax(p->getSize());

        SwapChainDesc sdesc = getSceneManager()->swapChain()->GetDesc();

        float scale = d->overlay_width;

        float aspect = (float)sdesc.Width / (float)sdesc.Height;

        float width  = size.x() / scale;
        float height = width / aspect;

        float l = -d->overlay_x*width;
        float t = -d->overlay_y*height;

        if(d->overlay_align_right)
            l = -(width-size.x())+d->overlay_x*width;

        if(d->overlay_align_bottom)
            t = -(height-size.y())+d->overlay_y*height;


        float r = l+width;
        float b = t+height;

        Eigen::Matrix4d m;
        m <<
            2.0f / (r - l),                  0.0f,   0.0f,   0.0f,
            0.0f,                  2.0f / (t - b),   0.0f,   0.0f,
            0.0f,                            0.0f,   0.5f,   0.0f,
            (r + l) / (l - r),  (t + b) / (b - t),   0.5f,   1.0f;

        m.transposeInPlace();

        d->matrices.view.setIdentity();
        d->matrices.proj = d->matrices.view;
        d->matrices.view_proj = d->matrices.view;
        d->matrices.world_view = d->matrices.view;
        d->matrices.world_view_proj = m;
    }

    for(const auto& p : d->layers)
        if(p->isVisible())
            p->render();



}



CanvasDrawingLayer* CanvasObject::createDrawingLayer()
{
    CanvasDrawingLayer* layer = new CanvasDrawingLayer(this);
    addLayer(std::unique_ptr<CanvasLayer>(layer));
    return layer;

}

CanvasImageLayer* CanvasObject::createImageLayer()
{
    CanvasImageLayer* layer = new CanvasImageLayer(this);
    addLayer(std::unique_ptr<CanvasLayer>(layer));
    return layer;
}

CanvasManualLayer* CanvasObject::createManualLayer()
{
    CanvasManualLayer* layer = new CanvasManualLayer(this);
    addLayer(std::unique_ptr<CanvasLayer>(layer));
    return layer;
}

void CanvasObject::addLayer(std::unique_ptr<CanvasLayer> layer)
{
    d->layers.push_back(std::move(layer));
}

SceneManager* CanvasObject::getSceneManager()
{
    return d->manager;
}

ImDrawListSharedData* CanvasObject::getDrawListSharedData()
{
    return d->imgui_shared_data;
}

ImGuiContext* CanvasObject::getImGuiContext()
{
    return d->imgui_impl->getImGuiContext();
}

ImDrawList* CanvasObject::createDrawList()
{
    ImDrawList* draw_list = new ImDrawList(getDrawListSharedData());
    draw_list->Flags |= ImDrawListFlags_AllowVtxOffset;
    return draw_list;
}

void CanvasObject::renderDrawList(ImDrawList* draw_list, float opacity)
{
    std::vector<ImDrawList*> draw_lists{draw_list};

    ImDrawData data;
    data.CmdLists = draw_lists.data();
    data.CmdListsCount = 1;
    data.TotalVtxCount = draw_list->VtxBuffer.Size;
    data.TotalIdxCount = draw_list->IdxBuffer.Size;
    data.DisplayPos = ImVec2(0.0f,0.0f);
    data.DisplaySize = ImVec2(10000.0f,10000.0f);
    data.FramebufferScale = ImVec2(1.0f, 1.0f);
    data.Valid = true;

    getSceneManager()->context()->SetStencilRef(getStencilId());

    dg::ImGuiImplDg::RenderParams params;
    params.world_view_proj = &d->matrices.world_view_proj;
    params.opacity = opacity;
    d->imgui_impl->render(getSceneManager()->context(), &data, params);
}

const SceneManager::Matrices& CanvasObject::getRenderMatrices()
{
    return d->matrices;
}

unsigned int CanvasObject::getStencilId()
{
    return d->stencil_id;
}

void CanvasObject::onAttached(Node* node)
{
    for(auto& p : d->layers)
        p->onAttached(node);
}

void CanvasObject::onDetached(Node* node)
{
    for(auto& p : d->layers)
        p->onDetached(node);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CanvasLayer::CanvasLayer(CanvasObject* parent_object) : parent_object_(parent_object)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}
