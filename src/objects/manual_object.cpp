
#include <dg/objects/manual_object.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/node.hpp>
#include <dg/scene/scene_manager.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>

namespace dg {


ManualObject::ManualObject(SceneManager* manager) : Object(type_id<ManualObject>()),
		_manager(manager)
{
}

ManualObject::~ManualObject()
{

}


void ManualObject::clear()
{
	_sections.clear();
}


void ManualObject::begin(IMaterial::Ptr material, PRIMITIVE_TOPOLOGY topology)
{
	begin(material, DepthStencilStateDesc(), topology);
}

void ManualObject::begin(IMaterial::Ptr material, DepthStencilStateDesc depthStencilDesc, PRIMITIVE_TOPOLOGY topology)
{
	_currentSection.reset(new Section);
	_currentSection->_primitiveTopology = topology;
	_currentSection->_material = material;
	_currentSection->_render_order = _render_order;
	_currentSection->_depthStencilDesc = depthStencilDesc;

	_currentSection->setPsoNeedsUpdate();

	_vertexSize = 0;
	_vertexCount = 0;
	_buf_ptr = _buf.data();
	_indexCount = 0;

}

void ManualObject::end()
{
	if(getNode())
		getNode()->attach(_currentSection.get());


	//std::cout << "END: " << _buf.size() << ", " << _vertexSize << ", " << _vertexCount << std::endl;
	//for(float d : _buf)
	//    std::cout << d << std::endl;


    BufferDesc VertBuffDesc;
    VertBuffDesc.Name          = "ManualObject vertex buffer";
    VertBuffDesc.Usage         = USAGE_STATIC;
    VertBuffDesc.BindFlags     = BIND_VERTEX_BUFFER;
    VertBuffDesc.uiSizeInBytes = _vertexSize*_vertexCount*sizeof(float);

    BufferData VBData;
    VBData.pData    = _buf.data();
    VBData.DataSize = VertBuffDesc.uiSizeInBytes;
    _manager->device()->CreateBuffer(VertBuffDesc, &VBData, &_currentSection->_vertexBuffer);

    BufferDesc IndBuffDesc;
	IndBuffDesc.Name          = "ManualObject index buffer";
	IndBuffDesc.Usage         = USAGE_STATIC;
	IndBuffDesc.BindFlags     = BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = _indexCount*sizeof(std::uint32_t);
	BufferData IBData;
	IBData.pData    = _idxbuf.data();
	IBData.DataSize = IndBuffDesc.uiSizeInBytes;
	_manager->device()->CreateBuffer(IndBuffDesc, &IBData, &_currentSection->_indexBuffer);

	_currentSection->_indexCount = _indexCount;
	//std::cout << "VERTEXCOUNT: " << _vertexCount << std::endl;
	//std::cout << "INDEXCOUNT: " << _indexCount << std::endl;

	// TODO:!!!
	_currentSection->_rasterizerDesc.CullMode = CULL_MODE_NONE;
	_currentSection->_rasterizerDesc.FrontCounterClockwise = true;

	//_currentSection->_depthStencilDesc.DepthEnable = true;
	//_currentSection->_depthStencilDesc.DepthWriteEnable = true;

	_sections.push_back(std::move(_currentSection));
}

void ManualObject::position(float x, float y, float z)
{
    if (!_currentSection)
        DG_THROW("You must call begin() before position()");

    if(_vertexCount==0)
    {
    	_currentSection->_inputLayout.push_back(LayoutElement{0, 0, 3, VT_FLOAT32, false});
    	_vertexSize += 3;
    }

    addVertex();

    *_buf_ptr++ = x;
    *_buf_ptr++ = y;
    *_buf_ptr++ = z;
}

void ManualObject::normal(float x, float y, float z)
{
    if (!_currentSection)
        DG_THROW("You must call begin() before normal()");

    if(_vertexCount==1)
    {
    	_currentSection->_inputLayout.push_back(LayoutElement{1, 0, 3, VT_FLOAT32, false});
    	_vertexSize += 3;
    }

    *_buf_ptr++ = x;
    *_buf_ptr++ = y;
    *_buf_ptr++ = z;
}

void ManualObject::color(const Color& col)
{
    if (!_currentSection)
        DG_THROW("You must call begin() before color()");

    if(_vertexCount==1)
    {
        _currentSection->_inputLayout.push_back(LayoutElement{2, 0, 4, VT_FLOAT32, false});
        _vertexSize += 4;
    }

    *_buf_ptr++ = col.r;
    *_buf_ptr++ = col.g;
    *_buf_ptr++ = col.b;
    *_buf_ptr++ = col.a;

}

void ManualObject::color(float r, float g, float b, float a)
{
    color(Color(r,g,b,a));
}

void ManualObject::textureCoord(float u, float v)
{
    if (!_currentSection)
        DG_THROW("You must call begin() before textureCoord()");

    if(_vertexCount==1)
    {
        _currentSection->_inputLayout.push_back(LayoutElement{3, 0, 2, VT_FLOAT32, false});
        _vertexSize += 2;
    }

    *_buf_ptr++ = u;
    *_buf_ptr++ = v;
}

void ManualObject::index(std::uint32_t idx)
{
	_indexCount++;
	if(_indexCount > _idxbuf.size())
		_idxbuf.resize(std::max<std::size_t>(_idxbuf.size()*2, 16));

	_idxbuf[_indexCount-1] = idx;
}

void ManualObject::triangle(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3)
{
	index(i1);
	index(i2);
	index(i3);
}

void ManualObject::onAttached(Node* node)
{
	// attach all section objects
	for(auto& s : _sections)
		node->attach(s.get());
}

void ManualObject::onDetached(Node* node)
{
	// detach all section objects
	for(auto& s : _sections)
		node->detach(s.get());
}

void ManualObject::addVertex()
{
	++_vertexCount;

	std::size_t size = _vertexCount*_vertexSize;
	if(size > _buf.size())
	{
		//< "RESIZE before: " << _buf.size() << std::endl;
		_buf.resize(std::max<std::size_t>(_buf.size()*2, 128));
		//std::cout << "RESIZE after: " << _buf.size() << std::endl;
	}

	_buf_ptr = _buf.data() + (_vertexCount-1) * _vertexSize;
}

}
