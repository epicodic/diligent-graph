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
        render_order_ = order;
    }


public:

    struct Section : public Renderable
    {
    friend class ManualObject;
    };

    typedef std::vector<std::unique_ptr<Section>> Sections;

    const Sections& getSections() { return sections_; }


protected:

	virtual void onAttached(Node* node) override;
	virtual void onDetached(Node* node) override;


private:

	void addVertex();

private:


	SceneManager* manager_;

	Sections sections_;
	std::unique_ptr<Section> currentSection_;

	std::size_t vertexSize_ = 0;
	std::size_t vertexCount_ = 0;
	float* buf_ptr_ = nullptr;
	std::vector<float> buf_;

	std::size_t indexCount_ = 0;
	std::vector<std::uint32_t> idxbuf_;

	RenderOrder render_order_;


};

}
