
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


static const char* g_pixel_shader_source = R"(
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
    RefCntAutoPtr<IBuffer>                vertex_constant_buffer;
    RefCntAutoPtr<IPipelineState>         pso;
    RefCntAutoPtr<ITextureView>           font_srv;
    RefCntAutoPtr<IShaderResourceBinding> srb;
    IShaderResourceVariable*              texture_var = nullptr;
    std::uint16_t                         back_buffer_fmt = 0;
    std::uint16_t                         depth_buffer_fmt = 0;
    std::uint32_t                         vertex_buffer_size = 0;
    std::uint32_t                         index_buffer_size  = 0;

    ImGuiContext* imgui_ctx = nullptr;
};



ImGuiImplDg::ImGuiImplDg(IRenderDevice* device,
                        std::uint16_t  back_buffer_fmt,
                        std::uint16_t  depth_buffer_fmt,
                        std::uint32_t  initial_vertex_buffer_size,
                        std::uint32_t  initial_index_buffer_size) : d(new Pimpl)
{
    d->device = device;
    d->back_buffer_fmt = back_buffer_fmt;
    d->depth_buffer_fmt = depth_buffer_fmt;
    d->vertex_buffer_size = initial_vertex_buffer_size;
    d->index_buffer_size = initial_index_buffer_size;


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
    d->vertex_constant_buffer.Release();
    d->pso.Release();
    d->font_srv.Release();
    d->srb.Release();
}

void ImGuiImplDg::createDeviceObjects()
{
    invalidateDeviceObjects();

    ShaderCreateInfo shader_ci;
    shader_ci.UseCombinedTextureSamplers = true;
    shader_ci.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;

    RefCntAutoPtr<IShader> vs;
    {
        shader_ci.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shader_ci.Desc.Name       = "Imgui VS";
        shader_ci.Source          = VertexShaderSource;
        d->device->CreateShader(shader_ci, &vs);
    }

    RefCntAutoPtr<IShader> ps;
    {
        shader_ci.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shader_ci.Desc.Name       = "Imgui PS";
        shader_ci.Source          = g_pixel_shader_source;
        d->device->CreateShader(shader_ci, &ps);
    }

    PipelineStateCreateInfo pso_create_info;
    PipelineStateDesc&      pso_desc = pso_create_info.PSODesc;

    pso_desc.Name           = "ImGUI PSO";
    auto& graphics_pipeline = pso_desc.GraphicsPipeline;

    graphics_pipeline.NumRenderTargets  = 1;
    graphics_pipeline.RTVFormats[0]     = (TEXTURE_FORMAT) d->back_buffer_fmt;
    graphics_pipeline.DSVFormat         = (TEXTURE_FORMAT) d->depth_buffer_fmt;
    graphics_pipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    graphics_pipeline.pVS = vs;
    graphics_pipeline.pPS = ps;

    graphics_pipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
    graphics_pipeline.RasterizerDesc.ScissorEnable = false; // TODO: True!
    graphics_pipeline.DepthStencilDesc.DepthEnable = true;
    graphics_pipeline.RasterizerDesc.DepthClipEnable = false; // TODO: Remove


    DepthStencilStateDesc depth_stencil_desc;
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthFunc = COMPARISON_FUNC_ALWAYS;
    depth_stencil_desc.StencilEnable = true;
    depth_stencil_desc.FrontFace.StencilPassOp = STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
    depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;


    graphics_pipeline.DepthStencilDesc = depth_stencil_desc;

    auto& RT0                 = graphics_pipeline.BlendDesc.RenderTargets[0];
    RT0.BlendEnable           = true;
    RT0.SrcBlend              = BLEND_FACTOR_SRC_ALPHA;
    RT0.DestBlend             = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.BlendOp               = BLEND_OPERATION_ADD;
    RT0.SrcBlendAlpha         = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.DestBlendAlpha        = BLEND_FACTOR_ZERO;
    RT0.BlendOpAlpha          = BLEND_OPERATION_ADD;
    RT0.RenderTargetWriteMask = COLOR_MASK_ALL;

    LayoutElement vs_inputs[] //
        {
            {0, 0, 2, VT_FLOAT32},    // pos
            {1, 0, 2, VT_FLOAT32},    // uv
            {2, 0, 4, VT_UINT8, true} // col
        };
    graphics_pipeline.InputLayout.NumElements    = 3;
    graphics_pipeline.InputLayout.LayoutElements = vs_inputs;

    ShaderResourceVariableDesc variables[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC} //
        };
    pso_desc.ResourceLayout.Variables    = variables;
    pso_desc.ResourceLayout.NumVariables = 1;

    SamplerDesc sam_linear_wrap;
    sam_linear_wrap.AddressU = TEXTURE_ADDRESS_WRAP;
    sam_linear_wrap.AddressV = TEXTURE_ADDRESS_WRAP;
    sam_linear_wrap.AddressW = TEXTURE_ADDRESS_WRAP;
    StaticSamplerDesc static_samplers[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", sam_linear_wrap} //
        };
    pso_desc.ResourceLayout.StaticSamplers    = static_samplers;
    pso_desc.ResourceLayout.NumStaticSamplers = 1;

    d->device->CreatePipelineState(pso_create_info, &d->pso);

    {
        BufferDesc buff_desc;
        buff_desc.uiSizeInBytes  = sizeof(ImGuiImplDgShaderConstants);
        buff_desc.Usage          = USAGE_DYNAMIC;
        buff_desc.BindFlags      = BIND_UNIFORM_BUFFER;
        buff_desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(buff_desc, nullptr, &d->vertex_constant_buffer);
    }
    d->pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(d->vertex_constant_buffer);

    updateFontsTexture();
}

void ImGuiImplDg::updateFontsTexture()
{
    // Build texture atlas
    ImGuiIO&       io     = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int            width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDesc font_tex_desc;
    font_tex_desc.Name      = "ImGuiImplDg font texture";
    font_tex_desc.Type      = RESOURCE_DIM_TEX_2D;
    font_tex_desc.Width     = static_cast<Uint32>(width);
    font_tex_desc.Height    = static_cast<Uint32>(height);
    font_tex_desc.Format    = TEX_FORMAT_RGBA8_UNORM;
    font_tex_desc.BindFlags = BIND_SHADER_RESOURCE;
    font_tex_desc.Usage     = USAGE_STATIC;

    TextureSubResData mip0_data[] = {{pixels, font_tex_desc.Width * 4}};
    TextureData       init_data(mip0_data, 1);

    RefCntAutoPtr<ITexture> font_tex;
    d->device->CreateTexture(font_tex_desc, &init_data, &font_tex);
    d->font_srv = font_tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    d->srb.Release();
    d->pso->CreateShaderResourceBinding(&d->srb, true);
    d->texture_var = d->srb->GetVariableByName(SHADER_TYPE_PIXEL, "Texture");
    VERIFY_EXPR(d->textureVar != nullptr);
    // Store our identifier
    io.Fonts->TexID = (ImTextureID)d->font_srv;
}

void ImGuiImplDg::render(IDeviceContext* ctx, ImDrawData* draw_data, const RenderParams& params)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Create and grow vertex/index buffers if needed
    if (!d->vb || static_cast<int>(d->vertex_buffer_size) < draw_data->TotalVtxCount)
    {
        d->vb.Release();
        while (static_cast<int>(d->vertex_buffer_size) < draw_data->TotalVtxCount)
            d->vertex_buffer_size *= 2;

        BufferDesc vb_desc;
        vb_desc.Name           = "Imgui vertex buffer";
        vb_desc.BindFlags      = BIND_VERTEX_BUFFER;
        vb_desc.uiSizeInBytes  = d->vertex_buffer_size * sizeof(ImDrawVert);
        vb_desc.Usage          = USAGE_DYNAMIC;
        vb_desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(vb_desc, nullptr, &d->vb);
    }

    if (!d->ib || static_cast<int>(d->index_buffer_size) < draw_data->TotalIdxCount)
    {
        d->ib.Release();
        while (static_cast<int>(d->index_buffer_size) < draw_data->TotalIdxCount)
            d->index_buffer_size *= 2;

        BufferDesc ib_desc;
        ib_desc.Name           = "Imgui index buffer";
        ib_desc.BindFlags      = BIND_INDEX_BUFFER;
        ib_desc.uiSizeInBytes  = d->index_buffer_size * sizeof(ImDrawIdx);
        ib_desc.Usage          = USAGE_DYNAMIC;
        ib_desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        d->device->CreateBuffer(ib_desc, nullptr, &d->ib);
    }

    {
        MapHelper<ImDrawVert> verices(ctx, d->vb, MAP_WRITE, MAP_FLAG_DISCARD);
        MapHelper<ImDrawIdx>  indices(ctx, d->ib, MAP_WRITE, MAP_FLAG_DISCARD);

        ImDrawVert* vtx_dst = verices;
        ImDrawIdx*  idx_dst = indices;
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
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
        MapHelper<ImGuiImplDgShaderConstants> cb_data(ctx, d->vertex_constant_buffer, MAP_WRITE, MAP_FLAG_DISCARD);

        cb_data->Opacity = params.opacity;

        if(params.world_view_proj == nullptr)
        {
            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

            //*CBData = float4x4
            cb_data->ProjectionMatrix = float4x4
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
            matrix_to_float4x4t(*params.world_view_proj, cb_data->ProjectionMatrix);
        }
    }

    auto display_width     = static_cast<Uint32>(draw_data->DisplaySize.x);
    auto display_height    = static_cast<Uint32>(draw_data->DisplaySize.y);
    auto setupRenderState = [&]() //
    {
        // Setup shader and vertex buffers
        Uint32   offsets[] = {0};
        IBuffer* vbs[]    = {d->vb};
        ctx->SetVertexBuffers(0, 1, vbs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        ctx->SetIndexBuffer(d->ib, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        ctx->SetPipelineState(d->pso);
        ctx->CommitShaderResources(d->srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
        ctx->SetBlendFactors(blend_factor);

        Viewport vp;
        vp.Width    = draw_data->DisplaySize.x;
        vp.Height   = draw_data->DisplaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        //ctx->SetViewports(1, &vp, DisplayWidth, DisplayHeight);
    };

    setupRenderState();

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ITextureView* last_texture_view = nullptr;

    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    setupRenderState();
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
                    d->texture_var->Set(texture_view);
                    ctx->CommitShaderResources(d->srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                DrawIndexedAttribs draw_attrs(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? VT_UINT16 : VT_UINT32, DRAW_FLAG_VERIFY_STATES);
                draw_attrs.FirstIndexLocation = pcmd->IdxOffset + global_idx_offset;
                draw_attrs.BaseVertex         = pcmd->VtxOffset + global_vtx_offset;

                ctx->DrawIndexed(draw_attrs);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}



}
