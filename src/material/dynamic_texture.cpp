#include <dg/material/dynamic_texture.hpp>

#include <vector>
//#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>


namespace dg {


void tex_convert(std::uint32_t  width, std::uint32_t  height, const void* src_data, std::uint32_t  src_stride, void* dest_data, std::uint32_t  dest_stride, DynamicTexture::DataFormat format)
{
    switch(format)
    {
        case DynamicTexture::GREY8:
        {
            const std::uint8_t* src  = (const std::uint8_t*)src_data;
            std::uint8_t* dest = (std::uint8_t*)dest_data;
            for(int v=0; v<height; ++v, src+=src_stride, dest+=dest_stride)
            for(int u=0; u<width;  ++u)
            {
                unsigned int val = src[u];
                dest[u*4+0] = val;
                dest[u*4+1] = val;
                dest[u*4+2] = val;
                dest[u*4+3] = 255;
            }
            break;
        }
        case DynamicTexture::GREY16:
        {
            const std::uint16_t* src  = (const std::uint16_t*)src_data;
            std::uint8_t* dest = (std::uint8_t*)dest_data;
            for(int v=0; v<height; ++v, src+=src_stride/2, dest+=dest_stride)
            for(int u=0; u<width;  ++u)
            {
                unsigned int val = src[u] / 16;
                dest[u*4+0] = val;
                dest[u*4+1] = val;
                dest[u*4+2] = val;
                dest[u*4+3] = 255;
            }
            break;
        }
        case DynamicTexture::RGB8:
        {
            const std::uint8_t* src  = (const std::uint8_t*)src_data;
            std::uint8_t* dest = (std::uint8_t*)dest_data;
            for(int v=0; v<height; ++v, src+=src_stride, dest+=dest_stride)
            for(int u=0; u<width;  ++u)
            {
                dest[u*4+0] = src[u*3+0];
                dest[u*4+1] = src[u*3+1];
                dest[u*4+2] = src[u*3+2];
                dest[u*4+3] = 255;
            }
            break;
        }
        case DynamicTexture::RGBA8:
        {
            const std::uint8_t* src  = (const std::uint8_t*)src_data;
            std::uint8_t* dest = (std::uint8_t*)dest_data;
            for(int v=0; v<height; ++v, src+=src_stride, dest+=dest_stride)
            for(int u=0; u<width;  ++u)
            {
                dest[u*4+0] = src[u*4+0];
                dest[u*4+1] = src[u*4+1];
                dest[u*4+2] = src[u*4+2];
                dest[u*4+3] = src[u*4+3];
            }
            break;
        }
        default:
            std::cout << "Unsupported format: " << format << std::endl;
            break;
    }
}


bool DynamicTexture::update(IRenderDevice *device, IDeviceContext *context, const ImageData& data)
{
    const TEXTURE_FORMAT tex_format = TEX_FORMAT_RGBA8_UNORM_SRGB;
    const int tex_bytes_per_pixel = 4; // TEX_FORMAT_RGBA8_UNORM_SRGB has 4 bytes per pixel

    if(!texture_) // CREATE
    {
        TextureDesc desc;

        desc.Name      = "DynamicTexture";
        desc.Type      = dg::RESOURCE_DIM_TEX_2D;
        desc.Width     = data.width;
        desc.Height    = data.height;
        desc.MipLevels = 1;
        desc.Usage          = dg::USAGE_DEFAULT;
        desc.BindFlags      = dg::BIND_SHADER_RESOURCE;
        desc.Format         = tex_format;
        desc.CPUAccessFlags = dg::CPU_ACCESS_WRITE; //CPU_ACCESS_NONE;


        buffer_.resize(data.width*data.height*tex_bytes_per_pixel);

        tex_convert(data.width, data.height, data.data, data.stride, buffer_.data(), data.width*tex_bytes_per_pixel, data.format);

        TextureSubResData sub_data;
        sub_data.pData = buffer_.data();
        sub_data.Stride = data.width*tex_bytes_per_pixel;

        TextureData tex_data;
        tex_data.pSubResources   = &sub_data;
        tex_data.NumSubresources = 1;

        device->CreateTexture(desc, &tex_data, &texture_);
    }
    else // UPDATE
    {
        const TextureDesc& desc = texture_->GetDesc();

        if(desc.Width != data.width || desc.Height != data.height)
            return false;

        buffer_.resize(data.width*data.height*tex_bytes_per_pixel);
        tex_convert(data.width,data.height,data.data,data.stride, buffer_.data(),data.width*tex_bytes_per_pixel, data.format);

        Box update_box;
        update_box.MinX = 0;
        update_box.MinY = 0;
        update_box.MaxX = data.width;
        update_box.MaxY = data.height;

        TextureSubResData subres_data;
        subres_data.Stride = data.width * tex_bytes_per_pixel;
        subres_data.pData  = buffer_.data();
        context->UpdateTexture(texture_, 0, 0, update_box, subres_data, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    return true;
}

}
