#pragma once

#include <memory>

#include <dg/core/fwds.hpp>
#include <dg/core/common.hpp>


// imgui forward decls
struct ImGuiContext;
struct ImDrawData;

namespace dg {

class ImGuiImplDg
{
public:
    static constexpr std::uint32_t DEFAULT_INITIAL_VB_SIZE = 1024;
    static constexpr std::uint32_t DEFAULT_INITIAL_IB_SIZE = 2048;

    ImGuiImplDg(IRenderDevice* device,
                std::uint16_t  back_buffer_fmt,
                std::uint16_t  depth_buffer_fmt,
                std::uint32_t  initial_vertex_buffer_size = DEFAULT_INITIAL_VB_SIZE,
                std::uint32_t  initial_index_buffer_size = DEFAULT_INITIAL_IB_SIZE);
    virtual ~ImGuiImplDg();

    ImGuiImplDg             (const ImGuiImplDg&)  = delete;
    ImGuiImplDg             (      ImGuiImplDg&&) = delete;
    ImGuiImplDg& operator = (const ImGuiImplDg&)  = delete;
    ImGuiImplDg& operator = (      ImGuiImplDg&&) = delete;


    ImGuiContext* getImGuiContext();

    struct RenderParams
    {
        const Eigen::Matrix4d* world_view_proj = nullptr;
        float opacity = 1.0f;
    };

    void render(IDeviceContext* ctx, ImDrawData* draw_data, const RenderParams& params);

public:
    // Use if you want to reset your rendering device without losing ImGui state.
    void invalidateDeviceObjects();
    void createDeviceObjects();
    void updateFontsTexture();

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> d;
};

}
