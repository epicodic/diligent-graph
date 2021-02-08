
#include <dg/gui/imgui_impl_dg.hpp>
#include <dg/core/conversion.hpp>

#include <memory>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>

#include <imgui/imgui.h>


namespace dg {

struct ImGuiImplDgShaderConstants
{
    float4x4 ProjectionMatrix;
    float Opacity;
};

static const char* VertexShaderSource = R"(
cbuffer Constants
{
    float4x4 ProjectionMatrix;
    float Opacity; 
}

struct VSInput
{
    float2 pos : ATTRIB0;
    float2 uv  : ATTRIB1;
    float4 col : ATTRIB2;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

void main(in VSInput VSIn, out PSInput PSIn)
{
    PSIn.pos = mul(ProjectionMatrix, float4(VSIn.pos.xy, 0.0, 1.0));
    PSIn.col = VSIn.col;
    PSIn.col.a = PSIn.col.a * Opacity;
    PSIn.uv  = VSIn.uv;
}
)";


static const char* PixelShaderSource = R"(
struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

Texture2D    Texture;
SamplerState Texture_sampler;

float4 main(in PSInput PSIn) : SV_Target
{
    return PSIn.col * Texture.Sample(Texture_sampler, PSIn.uv);
}
)";


struct ImGuiImplDg::Pimpl
{
    IRenderDevice* device = nullptr;
    RefCntAutoPtr<IBuffer>                vb;
    RefCntAutoPtr<IBuffer>                ib;
    RefCntAutoPtr<IBuffer>                vertexConstantBuffer;
    RefCntAutoPtr<IPipelineState>         pso;
    RefCntAutoPtr<ITextureView>           fontSRV;
    RefCntAutoPtr<IShaderResourceBinding> srb;
    IShaderResourceVariable*              textureVar = nullptr;
    std::uint16_t                         backBufferFmt = 0;
    std::uint16_t                         depthBufferFmt = 0;
    std::uint32_t                         vertexBufferSize = 0;
    std::uint32_t                         indexBufferSize  = 0;

    ImGuiContext* imgui_ctx = nullptr;
};



ImGuiImplDg::ImGuiImplDg(IRenderDevice* device,
                         std::uint16_t  backBufferFmt,
                         std::uint16_t  depthBufferFmt,
                         std::uint32_t  initialVertexBufferSize,
                         std::uint32_t  initialIndexBufferSize) : d(new Pimpl)
{
    d->device = device;
    d->backBufferFmt = backBufferFmt;
    d->depthBufferFmt = depthBufferFmt;
    d->vertexBufferSize = initialVertexBufferSize;
    d->indexBufferSize = initialIndexBufferSize;


    IMGUI_CHECKVERSION();
    ImGuiContext* oldctx = ImGui::GetCurrentContext();

    d->imgui_ctx = ImGui::CreateContext(oldctx ? ImGui::GetIO().Fonts : nullptr);

    ImGui::SetCurrentContext(d->imgui_ctx);

    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "ImGuiImplDg";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.IniFilename = nullptr;


    createDeviceObjects();

    ImGui::SetCurrentContext(oldctx);

}


ImGuiImplDg::~ImGuiImplDg()
{

}


ImGuiContext* ImGuiImplDg::getImGuiContext()
{
    return d->imgui_ctx;
}


void ImGuiImplDg::invalidateDeviceObjects()
{
    d->vb.Release();
    d->ib.Release();
    d->vertexConstantBuffer.Release();
    d->pso.Release();
    d->fontSRV.Release();
    d->srb.Release();
}

void ImGuiImplDg::createDeviceObjects()
{
    invalidateDeviceObjects();

    ShaderCreateInfo ShaderCI;
    ShaderCI.UseCombinedTextureSamplers = true;
    ShaderCI.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;

    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.Desc.Name       = "Imgui VS";
        ShaderCI.Source          = VertexShaderSource;
        d->device->CreateShader(ShaderCI, &pVS);
    }

    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.Desc.Name       = "Imgui PS";
        ShaderCI.Source          = PixelShaderSource;
        d->device->CreateShader(ShaderCI, &pPS);
    }

    PipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&      PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name           = "ImGUI PSO";
    auto& GraphicsPipeline = PSODesc.GraphicsPipeline;

    GraphicsPipeline.NumRenderTargets  = 1;
    GraphicsPipeline.RTVFormats[0]     = (TEXTURE_FORMAT) d->backBufferFmt;
    GraphicsPipeline.DSVFormat         = (TEXTURE_FORMAT) d->depthBufferFmt;
    GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    GraphicsPipeline.pVS = pVS;
    GraphicsPipeline.pPS = pPS;

    GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
    GraphicsPipeline.RasterizerDesc.ScissorEnable = false; // TODO: True!
    GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    GraphicsPipeline.RasterizerDesc.DepthClipEnable = false; // TODO: Remove


    DepthStencilStateDesc depthStencilDesc;
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthFunc = COMPARISON_FUNC_ALWAYS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.FrontFace.StencilPassOp = STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;


    GraphicsPipeline.DepthStencilDesc = depthStencilDesc;

    auto& RT0                 = GraphicsPipeline.BlendDesc.RenderTargets[0];
    RT0.BlendEnable           = true;
    RT0.SrcBlend              = BLEND_FACTOR_SRC_ALPHA;
    RT0.DestBlend             = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.BlendOp               = BLEND_OPERATION_ADD;
    RT0.SrcBlendAlpha         = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.DestBlendAlpha        = BLEND_FACTOR_ZERO;
    RT0.BlendOpAlpha          = BLEND_OPERATION_ADD;
    RT0.RenderTargetWriteMask = COLOR_MASK_ALL;

    LayoutElement VSInputs[] //
        {
            {0, 0, 2, VT_FLOAT32},    // pos
            {1, 0, 2, VT_FLOAT32},    // uv
            {2, 0, 4, VT_UINT8, true} // col
        };
    GraphicsPipeline.InputLayout.NumElements    = 3;
    GraphicsPipeline.InputLayout.LayoutElements = VSInputs;

    ShaderResourceVariableDesc Variables[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC} //
        };
    PSODesc.ResourceLayout.Variables    = Variables;
    PSODesc.ResourceLayout.NumVariables = 1;

    SamplerDesc SamLinearWrap;
    SamLinearWrap.AddressU = TEXTURE_ADDRESS_WRAP;
    SamLinearWrap.AddressV = TEXTURE_ADDRESS_WRAP;
    SamLinearWrap.AddressW = TEXTURE_ADDRESS_WRAP;
    StaticSamplerDesc StaticSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", SamLinearWrap} //
        };
    PSODesc.ResourceLayout.StaticSamplers    = StaticSamplers;
    PSODesc.ResourceLayout.NumStaticSamplers = 1;

    d->device->CreatePipelineState(PSOCreateInfo, &d->pso);

    {
        BufferDesc BuffDesc;
        BuffDesc.uiSizeInBytes  = sizeof(ImGuiImplDgShaderConstants);
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(BuffDesc, nullptr, &d->vertexConstantBuffer);
    }
    d->pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(d->vertexConstantBuffer);

    updateFontsTexture();
}

void ImGuiImplDg::updateFontsTexture()
{
    // Build texture atlas
    ImGuiIO&       io     = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int            width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDesc FontTexDesc;
    FontTexDesc.Name      = "ImGuiImplDg font texture";
    FontTexDesc.Type      = RESOURCE_DIM_TEX_2D;
    FontTexDesc.Width     = static_cast<Uint32>(width);
    FontTexDesc.Height    = static_cast<Uint32>(height);
    FontTexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
    FontTexDesc.BindFlags = BIND_SHADER_RESOURCE;
    FontTexDesc.Usage     = USAGE_STATIC;

    TextureSubResData Mip0Data[] = {{pixels, FontTexDesc.Width * 4}};
    TextureData       InitData(Mip0Data, 1);

    RefCntAutoPtr<ITexture> pFontTex;
    d->device->CreateTexture(FontTexDesc, &InitData, &pFontTex);
    d->fontSRV = pFontTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    d->srb.Release();
    d->pso->CreateShaderResourceBinding(&d->srb, true);
    d->textureVar = d->srb->GetVariableByName(SHADER_TYPE_PIXEL, "Texture");
    VERIFY_EXPR(d->textureVar != nullptr);
    // Store our identifier
    io.Fonts->TexID = (ImTextureID)d->fontSRV;
}

void ImGuiImplDg::render(IDeviceContext* ctx, ImDrawData* pDrawData, const RenderParams& params)
{
    // Avoid rendering when minimized
    if (pDrawData->DisplaySize.x <= 0.0f || pDrawData->DisplaySize.y <= 0.0f)
        return;

    // Create and grow vertex/index buffers if needed
    if (!d->vb || static_cast<int>(d->vertexBufferSize) < pDrawData->TotalVtxCount)
    {
        d->vb.Release();
        while (static_cast<int>(d->vertexBufferSize) < pDrawData->TotalVtxCount)
            d->vertexBufferSize *= 2;

        BufferDesc VBDesc;
        VBDesc.Name           = "Imgui vertex buffer";
        VBDesc.BindFlags      = BIND_VERTEX_BUFFER;
        VBDesc.uiSizeInBytes  = d->vertexBufferSize * sizeof(ImDrawVert);
        VBDesc.Usage          = USAGE_DYNAMIC;
        VBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(VBDesc, nullptr, &d->vb);
    }

    if (!d->ib || static_cast<int>(d->indexBufferSize) < pDrawData->TotalIdxCount)
    {
        d->ib.Release();
        while (static_cast<int>(d->indexBufferSize) < pDrawData->TotalIdxCount)
            d->indexBufferSize *= 2;

        BufferDesc IBDesc;
        IBDesc.Name           = "Imgui index buffer";
        IBDesc.BindFlags      = BIND_INDEX_BUFFER;
        IBDesc.uiSizeInBytes  = d->indexBufferSize * sizeof(ImDrawIdx);
        IBDesc.Usage          = USAGE_DYNAMIC;
        IBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(IBDesc, nullptr, &d->ib);
    }

    {
        MapHelper<ImDrawVert> Verices(ctx, d->vb, MAP_WRITE, MAP_FLAG_DISCARD);
        MapHelper<ImDrawIdx>  Indices(ctx, d->ib, MAP_WRITE, MAP_FLAG_DISCARD);

        ImDrawVert* vtx_dst = Verices;
        ImDrawIdx*  idx_dst = Indices;
        for (int n = 0; n < pDrawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = pDrawData->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
    }


    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from pDrawData->DisplayPos (top left) to pDrawData->DisplayPos+data_data->DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps.
    {
        //MapHelper<float4x4> CBData(ctx, d->vertexConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        MapHelper<ImGuiImplDgShaderConstants> CBData(ctx, d->vertexConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD);

        CBData->Opacity = params.opacity;

        if(params.worldViewProj == nullptr)
        {
            float L = pDrawData->DisplayPos.x;
            float R = pDrawData->DisplayPos.x + pDrawData->DisplaySize.x;
            float T = pDrawData->DisplayPos.y;
            float B = pDrawData->DisplayPos.y + pDrawData->DisplaySize.y;

            //*CBData = float4x4
            CBData->ProjectionMatrix = float4x4
            {
                2.0f / (R - L),                  0.0f,   0.0f,   0.0f,
                0.0f,                  2.0f / (T - B),   0.0f,   0.0f,
                0.0f,                            0.0f,   0.5f,   0.0f,
                (R + L) / (L - R),  (T + B) / (B - T),   0.5f,   1.0f
            };
        }
        else
        {
            //matrix_to_float4x4t(*params.worldViewProj, *CBData);
            matrix_to_float4x4t(*params.worldViewProj, CBData->ProjectionMatrix);
        }
    }

    auto DisplayWidth     = static_cast<Uint32>(pDrawData->DisplaySize.x);
    auto DisplayHeight    = static_cast<Uint32>(pDrawData->DisplaySize.y);
    auto SetupRenderState = [&]() //
    {
        // Setup shader and vertex buffers
        Uint32   Offsets[] = {0};
        IBuffer* pVBs[]    = {d->vb};
        ctx->SetVertexBuffers(0, 1, pVBs, Offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        ctx->SetIndexBuffer(d->ib, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        ctx->SetPipelineState(d->pso);
        ctx->CommitShaderResources(d->srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
        ctx->SetBlendFactors(blend_factor);

        Viewport vp;
        vp.Width    = pDrawData->DisplaySize.x;
        vp.Height   = pDrawData->DisplaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        //ctx->SetViewports(1, &vp, DisplayWidth, DisplayHeight);
    };

    SetupRenderState();

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ITextureView* last_texture_view = nullptr;

    ImVec2 clip_off = pDrawData->DisplayPos;
    for (int n = 0; n < pDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = pDrawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState();
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Apply scissor/clipping rectangle
                const Rect r =
                    {
                        static_cast<Int32>(pcmd->ClipRect.x - clip_off.x),
                        static_cast<Int32>(pcmd->ClipRect.y - clip_off.y),
                        static_cast<Int32>(pcmd->ClipRect.z - clip_off.x),
                        static_cast<Int32>(pcmd->ClipRect.w - clip_off.y) //
                    };
                //ctx->SetScissorRects(1, &r, DisplayWidth, DisplayHeight); // TODO:

                // Bind texture, Draw
                auto* texture_view = reinterpret_cast<ITextureView*>(pcmd->TextureId);
                VERIFY_EXPR(texture_view);
                if (texture_view != last_texture_view)
                {
                    last_texture_view = texture_view;
                    d->textureVar->Set(texture_view);
                    ctx->CommitShaderResources(d->srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                DrawIndexedAttribs DrawAttrs(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? VT_UINT16 : VT_UINT32, DRAW_FLAG_VERIFY_STATES);
                DrawAttrs.FirstIndexLocation = pcmd->IdxOffset + global_idx_offset;
                DrawAttrs.BaseVertex         = pcmd->VtxOffset + global_vtx_offset;

                ctx->DrawIndexed(DrawAttrs);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}



}
