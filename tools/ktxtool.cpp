#include <string.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>

#include <GL/gl.h>

/*
#define GL_RGBA32F            0x8814
#define GL_RGBA32UI           0x8D70
#define GL_RGBA32I            0x8D82
#define GL_RGB32F             0x8815
#define GL_RGB32UI            0x8D71
#define GL_RGB32I             0x8D83
#define GL_RGBA16F            0x881A
#define GL_RGBA16             0x805B
#define GL_RGBA16UI           0x8D76
#define GL_RGBA16_SNORM       0x8F9B
#define GL_RGBA16I            0x8D88
#define GL_RG32F              0x8230
#define GL_RG32UI             0x823C
#define GL_RG32I              0x823B
#define GL_DEPTH32F_STENCIL8  0x8CAD
#define GL_RGB10_A2           0x8059
#define GL_RGB10_A2UI         0x906F
#define GL_R11F_G11F_B10F     0x8C3A
#define GL_RGBA8              0x8058
#define GL_RGBA8UI            0x8D7C
#define GL_RGBA8_SNORM        0x8F97
#define GL_RGBA8I             0x8D8E
#define GL_RG16F              0x822F
#define GL_RG16               0x822C
#define GL_RG16UI             0x823A
#define GL_RG16_SNORM         0x8F99
#define GL_RG16I              0x8239
#define GL_R32F               0x822E
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_R32UI              0x8236
#define GL_R32I               0x8235
#define GL_DEPTH24_STENCIL8   0x88F0
#define GL_RG8                0x822B
#define GL_RG8UI              0x8238
#define GL_RG8_SNORM          0x8F95
#define GL_RG8I               0x8237
#define GL_R16F               0x822D
#define GL_DEPTH_COMPONENT16  0x81A5
#define GL_R16                0x822A
#define GL_R16UI              0x8234
#define GL_R16_SNORM          0x8F98
#define GL_R16I               0x8233
#define GL_R8                 0x8229
#define GL_R8UI               0x8232
#define GL_R8_SNORM           0x8F94
#define GL_R8I                0x8231
#define GL_RGB9_E5            0x8C3D


#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT        0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT       0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT       0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT       0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT       0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_COMPRESSED_RED_RGTC1                0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1         0x8DBC
#define GL_COMPRESSED_RG_RGTC2                 0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2          0x8DBE
#define GL_COMPRESSED_RGBA_BPTC_UNORM          0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM    0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT    0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT  0x8E8F

*/

struct KTX10Header
{
    std::uint32_t endianness;
    std::uint32_t gl_type;
    std::uint32_t gl_type_size;
    std::uint32_t gl_format;
    std::uint32_t gl_internal_format;
    std::uint32_t gl_base_internal_format;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint32_t number_of_array_elements;
    std::uint32_t number_of_faces;
    std::uint32_t number_of_mipmap_levels;
    std::uint32_t bytes_of_key_value_data;
};

void loadKTX(const std::vector<char>& d)
{
    static constexpr std::uint8_t KTX10FileIdentifier[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    const std::uint8_t*    data      = reinterpret_cast<const std::uint8_t*>(d.data());
    const auto             data_size = d.size();


    if (data_size >= 12 && memcmp(data, KTX10FileIdentifier, sizeof(KTX10FileIdentifier)) == 0)
    {
        data += sizeof(KTX10FileIdentifier);
        const KTX10Header& header = *reinterpret_cast<KTX10Header const*>(data);
        data += sizeof(KTX10Header);
        // Skip key value data
        data += header.bytes_of_key_value_data;

        std::cout << std::hex;

        std::cout << "endianness: " << header.endianness << std::endl;
        std::cout << "gl_type: " << header.gl_type << std::endl;
        std::cout << "gl_type_size: " << header.gl_type_size << std::endl;
        std::cout << "gl_format: " << header.gl_format << std::endl;
        std::cout << "gl_internal_format: " << header.gl_internal_format << std::endl;
        std::cout << "gl_base_internal_format: " << header.gl_base_internal_format << std::endl;
        std::cout << "width: " << header.width << std::endl;
        std::cout << "height: " << header.height << std::endl;
        std::cout << "depth: " << header.depth << std::endl;
        std::cout << "number_of_array_elements: " << header.number_of_array_elements << std::endl;
        std::cout << "number_of_faces: " << header.number_of_faces << std::endl;
        std::cout << "number_of_mipmap_levels: " << header.number_of_mipmap_levels << std::endl;
        std::cout << "bytes_of_key_value_data: " << header.bytes_of_key_value_data << std::endl;



/*
        TextureDesc TexDesc;
        TexDesc.Name   = TexLoadInfo.Name;
        TexDesc.Format = FindDiligentTextureFormat(Header.GLInternalFormat);
        if (TexDesc.Format == TEX_FORMAT_UNKNOWN)
            LOG_ERROR_AND_THROW("Failed to find appropriate Diligent format for internal gl format ", Header.GLInternalFormat);
        TexDesc.Width     = Header.Width;
        TexDesc.Height    = std::max(Header.Height, 1u);
        TexDesc.Depth     = std::max(Header.Depth, 1u);
        TexDesc.MipLevels = std::max(Header.NumberOfMipmapLevels, 1u);
        TexDesc.BindFlags = TexLoadInfo.BindFlags;
        TexDesc.Usage     = TexLoadInfo.Usage;
        auto NumFaces     = std::max(Header.NumberOfFaces, 1u);
        if (NumFaces == 6)
        {
            TexDesc.ArraySize = std::max(Header.NumberOfArrayElements, 1u) * NumFaces;
            TexDesc.Type      = TexDesc.ArraySize > 6 ? RESOURCE_DIM_TEX_CUBE_ARRAY : RESOURCE_DIM_TEX_CUBE;
        }
        else
        {
            if (TexDesc.Depth > 1)
            {
                TexDesc.ArraySize = 1;
                TexDesc.Type      = RESOURCE_DIM_TEX_3D;
            }
            else
            {
                TexDesc.ArraySize = std::max(Header.NumberOfArrayElements, 1u);
                TexDesc.Type      = TexDesc.ArraySize > 1 ? RESOURCE_DIM_TEX_2D_ARRAY : RESOURCE_DIM_TEX_2D;
            }
        }

        auto                           ArraySize = (TexDesc.Type != RESOURCE_DIM_TEX_3D ? TexDesc.ArraySize : 1);
        std::vector<TextureSubResData> SubresData(TexDesc.MipLevels * ArraySize);
        for (std::uint32_t mip = 0; mip < TexDesc.MipLevels; ++mip)
        {
            data += sizeof(std::uint32_t);
            auto MipInfo = GetMipLevelProperties(TexDesc, mip);

            for (std::uint32_t layer = 0; layer < ArraySize; ++layer)
            {
                SubresData[mip + layer * TexDesc.MipLevels] =
                    TextureSubResData{data, MipInfo.RowSize, MipInfo.DepthSliceSize};
                data += Align(MipInfo.MipSize, 4u);
            }
        }
        VERIFY(data - reinterpret_cast<const std::uint8_t*>(pKTXData->GetDataPtr()) == static_cast<ptrdiff_t>(data_size), "Unexpected data size");

        TextureData InitData(SubresData.data(), static_cast<std::uint32_t>(SubresData.size()));
        pDevice->CreateTexture(TexDesc, &InitData, ppTexture);
        */
    }
    else
    {
        std::cout << "ktx2.0 is not currently supported" << std::endl;
        return;
    }
}


int main(int argc, char** argv)
{
	std::ifstream file(argv[1], std::ios::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)),
	                        std::istreambuf_iterator<char>());

	loadKTX(data);
}
