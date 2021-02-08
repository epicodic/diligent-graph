#include <dg/objects/canvas_image_layer.hpp>

#include <dg/material/material.hpp>

namespace dg {

CanvasImageLayer::CanvasImageLayer(CanvasObject* parent_object) : CanvasLayer(parent_object)
{
}

void CanvasImageLayer::setScale(float scale) 
{
    scale_ = scale;
	manualObject_.reset();
}

float CanvasImageLayer::getScale() const
{
    return scale_;
}

int borderEdge(dg::ManualObject* obj, float sx, float sy, float ox, float oy, int voff, float d, float w, float l, const Color& c0, const Color& c1)
{
    obj->position(ox+sx*(-d), oy+sy*( l), 0.0f); obj->color(c1);
    obj->position(ox+sx*(-d), oy+sy*(-d), 0.0f); obj->color(c1);
	obj->position(ox+sx*( l), oy+sy*(-d), 0.0f); obj->color(c1);


    obj->position(ox+sx*(-d-w), oy+sy*( l+w), 0.0f); obj->color(c0);
    obj->position(ox+sx*(-d-w), oy+sy*(-d-w), 0.0f); obj->color(c0);
	obj->position(ox+sx*( l+w), oy+sy*(-d-w), 0.0f); obj->color(c0);

    obj->position(ox+sx*(-d+w), oy+sy*( l+w), 0.0f); obj->color(c0);
    obj->position(ox+sx*(-d+w), oy+sy*(-d+w), 0.0f); obj->color(c0);
	obj->position(ox+sx*( l+w), oy+sy*(-d+w), 0.0f); obj->color(c0);

    obj->triangle(voff+0,voff+3,voff+6);
    obj->triangle(voff+0,voff+6,voff+7);
    obj->triangle(voff+0,voff+7,voff+1);
    obj->triangle(voff+0,voff+1,voff+3);

    obj->triangle(voff+1,voff+4,voff+3);
    obj->triangle(voff+1,voff+5,voff+4);

    obj->triangle(voff+2,voff+5,voff+1);
    obj->triangle(voff+2,voff+1,voff+7);
    obj->triangle(voff+2,voff+7,voff+8);
    obj->triangle(voff+2,voff+8,voff+5);

    return 9;
}


void CanvasImageLayer::update(const dg::DynamicTexture::ImageData& data)
{
	texture_.update(getParentObject()->getSceneManager()->device(), getParentObject()->getSceneManager()->context(), data);

	if(!manualObject_)
	{
		manualObject_ = dg::ManualObject::make_unique(getParentObject()->getSceneManager());
		material_ = dg::UnlitMaterial::make(getParentObject()->getSceneManager()->device());
		material_->texture = texture_.getTexture();
		material_->cull_mode = dg::material::RasterizerParams::CullMode::None;
		material_->initialize(getParentObject()->getSceneManager()->device()); // TODO: this should be done automatically

		float width = data.width * scale_;
		float height = data.height * scale_;
		_size << width, height;

		float m_far_plane = 1.0;

	    manualObject_->clear();

	    DepthStencilStateDesc depth_stencil_desc;

	    depth_stencil_desc.DepthEnable = true;
	    if(getParentObject()->isOverlayMode()) // TODO: depth/stencil desc must become part of material
	    	depth_stencil_desc.DepthEnable = false;

	    depth_stencil_desc.StencilEnable = true;
	    depth_stencil_desc.FrontFace.StencilPassOp = STENCIL_OP_REPLACE;
	    depth_stencil_desc.FrontFace.StencilFunc = COMPARISON_FUNC_ALWAYS;
	    depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;

	    material_->color = Color(1.0f,1.0f,1.0f,1.0f);

	    manualObject_->begin(material_, depth_stencil_desc, dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	    manualObject_->position(0.0f,0.0f,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(0.0f,0.0f);

	    manualObject_->position(width,0.0f,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(1.0f,0.0f);

	    manualObject_->position(width,height,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(1.0f,1.0f);

	    manualObject_->index(0);
	    manualObject_->index(1);
	    manualObject_->index(2);

	    manualObject_->position(width,height,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(1.0f,1.0f);

	    manualObject_->position(0.0f, height,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(0.0f,1.0f);

	    manualObject_->position(0.0f,0.0f,0.0f);
	    manualObject_->color(dg::colors::White);
	    manualObject_->textureCoord(0.0f,0.0f);

	    manualObject_->index(3);
	    manualObject_->index(4);
	    manualObject_->index(5);

	    manualObject_->end();


	    material_border_ = dg::UnlitMaterial::make(getParentObject()->getSceneManager()->device());

	    RenderTargetBlendDesc blend_desc;
		blend_desc.BlendEnable = true;
		blend_desc.RenderTargetWriteMask = COLOR_MASK_ALL;
		blend_desc.SrcBlend       = BLEND_FACTOR_SRC_ALPHA;
		blend_desc.DestBlend      = BLEND_FACTOR_ONE;
		blend_desc.BlendOp        = BLEND_OPERATION_ADD;
		blend_desc.SrcBlendAlpha  = BLEND_FACTOR_ZERO;
		blend_desc.DestBlendAlpha = BLEND_FACTOR_ONE;
		blend_desc.BlendOpAlpha   = BLEND_OPERATION_ADD;
		material_border_->setBlendDesc(blend_desc);
		material_border_->cull_mode = dg::material::RasterizerParams::CullMode::None;

		float _border_spacing = 20.0f;
		float _width = 15.0f;
		float _length = 50.0f;

		float  d = _border_spacing;
		float  w = _width/2.0f;
		float  l = _length;

		//Color c0(0.0,0.0,0.0,0.0);
		Color c0(0.2,0.5,0.5,1.0);
		Color c1(0.2,0.5,0.5,1.0);

	    manualObject_->begin(material_border_, dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	    int voff;
	    voff += borderEdge(manualObject_.get(),  1.0f,  1.0,  0.0f,   0.0f, voff, d, w, l, c0, c1);
	    voff += borderEdge(manualObject_.get(), -1.0f,  1.0, width,   0.0f, voff, d, w, l, c0, c1);
	    voff += borderEdge(manualObject_.get(),  1.0f, -1.0,  0.0f, height, voff, d, w, l, c0, c1);
	    voff += borderEdge(manualObject_.get(), -1.0f, -1.0, width, height, voff, d, w, l, c0, c1);

	    manualObject_->end();

	}
}

void CanvasImageLayer::render()
{
	if(!manualObject_)
		return;

	if(getParentObject()->isOverlayMode())
		material_border_->opacity = 0.0f;

	getParentObject()->getSceneManager()->context()->SetStencilRef(getParentObject()->getStencilId());

	for(const auto& p : manualObject_->getSections())
		getParentObject()->getSceneManager()->render(p.get(), getParentObject()->getRenderMatrices());
}

void CanvasImageLayer::setOpacity(float opacity)
{
	if(material_)
	{
		material_->opacity = opacity;
		material_border_->opacity = opacity;
	}
}

}
