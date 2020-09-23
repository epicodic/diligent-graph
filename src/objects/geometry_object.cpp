
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

	std::size_t vertexSize = 0;

	std::size_t vertexCount = std::max(positions.size(), normals.size());

	if(!positions.empty())
	{
		if(positions.size()!=vertexCount)
			DG_THROW("Number of positions does not match count of other items");

		_inputLayout.push_back(LayoutElement{0, 0, 3, VT_FLOAT32, false});
		vertexSize += 3;
	}

	if(!normals.empty())
	{
		if(normals.size()!=vertexCount)
			DG_THROW("Number of normals does not match count of other items");

		_inputLayout.push_back(LayoutElement{1, 0, 3, VT_FLOAT32, false});
		vertexSize += 3;
	}

	if(!colors.empty())
	{
		if(colors.size()!=vertexCount)
			DG_THROW("Number of colors does not match count of other items");

		_inputLayout.push_back(LayoutElement{2, 0, 4, VT_FLOAT32, false});
		vertexSize += 4;
	}

	if(!uvs.empty())
	{
		if(uvs.size()!=vertexCount)
			DG_THROW("Number of uv texture coordinates does not match count of other items");

		_inputLayout.push_back(LayoutElement{3, 0, 2, VT_FLOAT32, false});
		vertexSize += 2;
	}
	
	std::vector<float> vbuf(vertexSize*vertexCount);

	std::size_t idx=0;
	for(std::size_t i=0; i<vertexCount; ++i)
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

	BufferDesc VertBuffDesc;
	VertBuffDesc.Name          = "GeometryObject vertex buffer";
	VertBuffDesc.Usage         = USAGE_STATIC;
	VertBuffDesc.BindFlags     = BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = vertexSize*vertexCount*sizeof(float);

	BufferData VBData;
	VBData.pData    = vbuf.data();
	VBData.DataSize = VertBuffDesc.uiSizeInBytes;
	manager->device()->CreateBuffer(VertBuffDesc, &VBData, &_vertexBuffer);


	_indexCount = indices.size();

	BufferDesc IndBuffDesc;
	IndBuffDesc.Name          = "GeometryObject index buffer";
	IndBuffDesc.Usage         = USAGE_STATIC;
	IndBuffDesc.BindFlags     = BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = _indexCount*sizeof(std::uint32_t);
	BufferData IBData;
	IBData.pData    = indices.data();
	IBData.DataSize = IndBuffDesc.uiSizeInBytes;
	manager->device()->CreateBuffer(IndBuffDesc, &IBData, &_indexBuffer);

	_rasterizerDesc.CullMode = CULL_MODE_BACK;
	_rasterizerDesc.FrontCounterClockwise = true;

	_primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void GeometryObject::setMaterial(IMaterial::Ptr m)
{
	_material = m;
	setPsoNeedsUpdate();
}


}
