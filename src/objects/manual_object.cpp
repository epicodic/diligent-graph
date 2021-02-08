
#include <dg/objects/manual_object.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/node.hpp>
#include <dg/scene/scene_manager.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>

namespace dg {


ManualObject::ManualObject(SceneManager* manager) : Object(type_id<ManualObject>()),
		manager_(manager)
{
}

ManualObject::~ManualObject()
{

}


void ManualObject::clear()
{
	sections_.clear();
}


void ManualObject::begin(IMaterial::Ptr material, PRIMITIVE_TOPOLOGY topology)
{
	begin(material, DepthStencilStateDesc(), topology);
}

void ManualObject::begin(IMaterial::Ptr material, DepthStencilStateDesc depthStencilDesc, PRIMITIVE_TOPOLOGY topology)
{
	currentSection_.reset(new Section);
	currentSection_->primitive_topology = topology;
	currentSection_->material = material;
	currentSection_->render_order = render_order_;
	currentSection_->depth_stencil_desc = depthStencilDesc;

	currentSection_->setPsoNeedsUpdate();

	vertexSize_ = 0;
	vertexCount_ = 0;
	buf_ptr_ = buf_.data();
	indexCount_ = 0;

}

void ManualObject::end()
{
	if(getNode())
		getNode()->attach(currentSection_.get());


	//std::cout << "END: " << _buf.size() << ", " << _vertexSize << ", " << _vertexCount << std::endl;
	//for(float d : _buf)
	//    std::cout << d << std::endl;


    BufferDesc vert_buff_desc;
    vert_buff_desc.Name          = "ManualObject vertex buffer";
    vert_buff_desc.Usage         = USAGE_STATIC;
    vert_buff_desc.BindFlags     = BIND_VERTEX_BUFFER;
    vert_buff_desc.uiSizeInBytes = vertexSize_*vertexCount_*sizeof(float);

    BufferData vb_data;
    vb_data.pData    = buf_.data();
    vb_data.DataSize = vert_buff_desc.uiSizeInBytes;
    manager_->device()->CreateBuffer(vert_buff_desc, &vb_data, &currentSection_->vertex_buffer);

    BufferDesc ind_buff_desc;
	ind_buff_desc.Name          = "ManualObject index buffer";
	ind_buff_desc.Usage         = USAGE_STATIC;
	ind_buff_desc.BindFlags     = BIND_INDEX_BUFFER;
	ind_buff_desc.uiSizeInBytes = indexCount_*sizeof(std::uint32_t);
	BufferData ib_data;
	ib_data.pData    = idxbuf_.data();
	ib_data.DataSize = ind_buff_desc.uiSizeInBytes;
	manager_->device()->CreateBuffer(ind_buff_desc, &ib_data, &currentSection_->index_buffer);

	currentSection_->index_count = indexCount_;
	//std::cout << "VERTEXCOUNT: " << _vertexCount << std::endl;
	//std::cout << "INDEXCOUNT: " << _indexCount << std::endl;

	// TODO:!!!
	currentSection_->rasterizer_desc.CullMode = CULL_MODE_NONE;
	currentSection_->rasterizer_desc.FrontCounterClockwise = true;

	//_currentSection->_depthStencilDesc.DepthEnable = true;
	//_currentSection->_depthStencilDesc.DepthWriteEnable = true;

	sections_.push_back(std::move(currentSection_));
}

void ManualObject::position(float x, float y, float z)
{
    if (!currentSection_)
        DG_THROW("You must call begin() before position()");

    if(vertexCount_==0)
    {
    	currentSection_->input_layout.push_back(LayoutElement{0, 0, 3, VT_FLOAT32, false});
    	vertexSize_ += 3;
    }

    addVertex();

    *buf_ptr_++ = x;
    *buf_ptr_++ = y;
    *buf_ptr_++ = z;
}

void ManualObject::normal(float x, float y, float z)
{
    if (!currentSection_)
        DG_THROW("You must call begin() before normal()");

    if(vertexCount_==1)
    {
    	currentSection_->input_layout.push_back(LayoutElement{1, 0, 3, VT_FLOAT32, false});
    	vertexSize_ += 3;
    }

    *buf_ptr_++ = x;
    *buf_ptr_++ = y;
    *buf_ptr_++ = z;
}

void ManualObject::color(const Color& col)
{
    if (!currentSection_)
        DG_THROW("You must call begin() before color()");

    if(vertexCount_==1)
    {
        currentSection_->input_layout.push_back(LayoutElement{2, 0, 4, VT_FLOAT32, false});
        vertexSize_ += 4;
    }

    *buf_ptr_++ = col.r;
    *buf_ptr_++ = col.g;
    *buf_ptr_++ = col.b;
    *buf_ptr_++ = col.a;

}

void ManualObject::color(float r, float g, float b, float a)
{
    color(Color(r,g,b,a));
}

void ManualObject::textureCoord(float u, float v)
{
    if (!currentSection_)
        DG_THROW("You must call begin() before textureCoord()");

    if(vertexCount_==1)
    {
        currentSection_->input_layout.push_back(LayoutElement{3, 0, 2, VT_FLOAT32, false});
        vertexSize_ += 2;
    }

    *buf_ptr_++ = u;
    *buf_ptr_++ = v;
}

void ManualObject::index(std::uint32_t idx)
{
	indexCount_++;
	if(indexCount_ > idxbuf_.size())
		idxbuf_.resize(std::max<std::size_t>(idxbuf_.size()*2, 16));

	idxbuf_[indexCount_-1] = idx;
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
	for(auto& s : sections_)
		node->attach(s.get());
}

void ManualObject::onDetached(Node* node)
{
	// detach all section objects
	for(auto& s : sections_)
		node->detach(s.get());
}

void ManualObject::addVertex()
{
	++vertexCount_;

	std::size_t size = vertexCount_*vertexSize_;
	if(size > buf_.size())
	{
		//< "RESIZE before: " << _buf.size() << std::endl;
		buf_.resize(std::max<std::size_t>(buf_.size()*2, 128));
		//std::cout << "RESIZE after: " << _buf.size() << std::endl;
	}

	buf_ptr_ = buf_.data() + (vertexCount_-1) * vertexSize_;
}

}
