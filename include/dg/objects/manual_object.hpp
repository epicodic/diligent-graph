#pragma once

#include <memory>

#include <dg/scene/renderable.hpp>
#include <dg/material/color.hpp>

namespace dg {

class ManualObject : public Object
{
public:
	DG_PTR(ManualObject)

	ManualObject(SceneManager* manager);
	virtual ~ManualObject();

public:

    void clear();

	void begin(IMaterial::Ptr material, PRIMITIVE_TOPOLOGY topology);
	void begin(IMaterial::Ptr material, DepthStencilStateDesc depthStencilDesc, PRIMITIVE_TOPOLOGY topology);


	void end();

	void position(float x, float y, float z);
	void normal(float x, float y, float z);
	void color(const Color& col);
	void color(float r, float g, float b, float a);
	void textureCoord(float u, float v);

	void index(std::uint32_t idx);
	void triangle(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3);

    void setRenderOrder(RenderOrder order)
    {
        _render_order = order;
    }


public:

    struct Section : public Renderable
    {
    friend class ManualObject;
    };

    typedef std::vector<std::unique_ptr<Section>> Sections;

    const Sections& getSections() { return _sections; }


protected:

	virtual void onAttached(Node* node) override;
	virtual void onDetached(Node* node) override;


private:

	void addVertex();

private:


	SceneManager* _manager;

	Sections _sections;
	std::unique_ptr<Section> _currentSection;

	std::size_t _vertexSize = 0;
	std::size_t _vertexCount = 0;
	float* _buf_ptr = nullptr;
	std::vector<float> _buf;

	std::size_t _indexCount = 0;
	std::vector<std::uint32_t> _idxbuf;

	RenderOrder _render_order;


};

}
