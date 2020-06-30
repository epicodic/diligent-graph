
#include <dg/objects/geometry_object.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/node.hpp>
#include <dg/scene/scene_manager.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>

namespace dg {

GeometryObject::GeometryObject(SceneManager* manager) :
		_manager(manager)
{
}

GeometryObject::~GeometryObject()
{

}

void GeometryObject::setMaterial(IMaterial::Ptr m)
{
	_material = m;
	setPsoNeedsUpdate();
}


void GeometryObject::generate()
{
	std::size_t vertexSize = 0;

	std::size_t vertexCount = std::max(_positions.size(), _normals.size());

	if(!_positions.empty())
	{
		if(_positions.size()!=vertexCount)
			DG_THROW("Number of positions does not match count of other items");

		_inputLayout.push_back(LayoutElement{0, 0, 3, VT_FLOAT32, false});
		vertexSize += 3;
	}

	if(!_normals.empty())
	{
		if(_normals.size()!=vertexCount)
			DG_THROW("Number of normals does not match count of other items");

		_inputLayout.push_back(LayoutElement{1, 0, 3, VT_FLOAT32, false});
		vertexSize += 3;
	}

	std::vector<float> vbuf(vertexSize*vertexCount);

	std::size_t idx=0;
	for(std::size_t i=0; i<vertexCount; ++i)
	{
		if(!_positions.empty())
		{
			vbuf[idx++] = _positions[i].x();
			vbuf[idx++] = _positions[i].y();
			vbuf[idx++] = _positions[i].z();
		}
		if(!_normals.empty())
		{
			vbuf[idx++] = _normals[i].x();
			vbuf[idx++] = _normals[i].y();
			vbuf[idx++] = _normals[i].z();
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
	_manager->device()->CreateBuffer(VertBuffDesc, &VBData, &_vertexBuffer);


	_indexCount = _indices.size();

	BufferDesc IndBuffDesc;
	IndBuffDesc.Name          = "GeometryObject index buffer";
	IndBuffDesc.Usage         = USAGE_STATIC;
	IndBuffDesc.BindFlags     = BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = _indexCount*sizeof(std::uint32_t);
	BufferData IBData;
	IBData.pData    = _indices.data();
	IBData.DataSize = IndBuffDesc.uiSizeInBytes;
	_manager->device()->CreateBuffer(IndBuffDesc, &IBData, &_indexBuffer);

	_rasterizerDesc.CullMode = CULL_MODE_BACK;
	_rasterizerDesc.FrontCounterClockwise = true;
}

}
