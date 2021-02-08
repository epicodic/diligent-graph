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
    static constexpr std::uint32_t DefaultInitialVBSize = 1024;
    static constexpr std::uint32_t DefaultInitialIBSize = 2048;

    ImGuiImplDg(IRenderDevice* device,
                std::uint16_t  backBufferFmt,
                std::uint16_t  depthBufferFmt,
                std::uint32_t  initialVertexBufferSize = DefaultInitialVBSize,
                std::uint32_t  initialIndexBufferSize  = DefaultInitialIBSize);
    virtual ~ImGuiImplDg();

    ImGuiImplDg             (const ImGuiImplDg&)  = delete;
    ImGuiImplDg             (      ImGuiImplDg&&) = delete;
    ImGuiImplDg& operator = (const ImGuiImplDg&)  = delete;
    ImGuiImplDg& operator = (      ImGuiImplDg&&) = delete;


    ImGuiContext* getImGuiContext();


    //virtual void newFrame();
    //virtual void endFrame();
    //virtual void render(IDeviceContext* pCtx);


    struct RenderParams
    {
        const Eigen::Matrix4d* worldViewProj = nullptr;
        float opacity = 1.0f;
    };

    void render(IDeviceContext* ctx, ImDrawData* pDrawData, const RenderParams& params);

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
