
#include <dg/objects/geometry_object.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/node.hpp>
#include <dg/scene/scene_manager.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>

namespace dg {

GeometryObject::GeometryObject(SceneManager* manager, const IGeometry& geometry, const std::vector<Color>& colors)
{
    const std::vector<Vector3>& positions = geometry.getPositions();
    const std::vector<Vector3>& normals = geometry.getNormals();
    const std::vector<Vector2f>& uvs = geometry.getUVs();
    const std::vector<std::uint32_t>& indices = geometry.getIndices();

    std::size_t vertex_size = 0;

    std::size_t vertex_count = std::max(positions.size(), normals.size());

    if(!positions.empty())
    {
        if(positions.size()!=vertex_count)
            DG_THROW("Number of positions does not match count of other items");

        input_layout.push_back(LayoutElement{0, 0, 3, VT_FLOAT32, false});
        vertex_size += 3;
    }

    if(!normals.empty())
    {
        if(normals.size()!=vertex_count)
            DG_THROW("Number of normals does not match count of other items");

        input_layout.push_back(LayoutElement{1, 0, 3, VT_FLOAT32, false});
        vertex_size += 3;
    }

    if(!colors.empty())
    {
        if(colors.size()!=vertex_count)
            DG_THROW("Number of colors does not match count of other items");

        input_layout.push_back(LayoutElement{2, 0, 4, VT_FLOAT32, false});
        vertex_size += 4;
    }

    if(!uvs.empty())
    {
        if(uvs.size()!=vertex_count)
            DG_THROW("Number of uv texture coordinates does not match count of other items");

        input_layout.push_back(LayoutElement{3, 0, 2, VT_FLOAT32, false});
        vertex_size += 2;
    }
    
    std::vector<float> vbuf(vertex_size*vertex_count);

    std::size_t idx=0;
    for(std::size_t i=0; i<vertex_count; ++i)
    {
        if(!positions.empty())
        {
            vbuf[idx++] = positions[i].x();
            vbuf[idx++] = positions[i].y();
            vbuf[idx++] = positions[i].z();
        }
        if(!normals.empty())
        {
            vbuf[idx++] = normals[i].x();
            vbuf[idx++] = normals[i].y();
            vbuf[idx++] = normals[i].z();
        }

        if(!colors.empty())
        {
            vbuf[idx++] = colors[i].r;
            vbuf[idx++] = colors[i].g;
            vbuf[idx++] = colors[i].b;
            vbuf[idx++] = colors[i].a;
        }

        if(!uvs.empty())
        {
            vbuf[idx++] = uvs[i].x();
            vbuf[idx++] = uvs[i].y();
        }
    }

    BufferDesc vert_buff_desc;
    vert_buff_desc.Name          = "GeometryObject vertex buffer";
    vert_buff_desc.Usage         = USAGE_STATIC;
    vert_buff_desc.BindFlags     = BIND_VERTEX_BUFFER;
    vert_buff_desc.uiSizeInBytes = vertex_size*vertex_count*sizeof(float);

    BufferData vb_data;
    vb_data.pData    = vbuf.data();
    vb_data.DataSize = vert_buff_desc.uiSizeInBytes;
    manager->device()->CreateBuffer(vert_buff_desc, &vb_data, &vertex_buffer);


    index_count = indices.size();

    BufferDesc ind_buff_desc;
    ind_buff_desc.Name          = "GeometryObject index buffer";
    ind_buff_desc.Usage         = USAGE_STATIC;
    ind_buff_desc.BindFlags     = BIND_INDEX_BUFFER;
    ind_buff_desc.uiSizeInBytes = index_count*sizeof(std::uint32_t);
    BufferData ib_data;
    ib_data.pData    = indices.data();
    ib_data.DataSize = ind_buff_desc.uiSizeInBytes;
    manager->device()->CreateBuffer(ind_buff_desc, &ib_data, &index_buffer);

    rasterizer_desc.CullMode = CULL_MODE_BACK;
    rasterizer_desc.FrontCounterClockwise = true;

    primitive_topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void GeometryObject::setMaterial(IMaterial::Ptr m)
{
    material = m;
    setPsoNeedsUpdate();
}


}
