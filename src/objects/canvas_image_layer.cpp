#include <dg/objects/canvas_image_layer.hpp>

namespace dg {

CanvasImageLayer::CanvasImageLayer(CanvasObject* parent_object) : CanvasLayer(parent_object)
{
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
	_texture.update(getParentObject()->getSceneManager()->device(), getParentObject()->getSceneManager()->context(), data);

	if(!_manualObject)
	{
		_manualObject = dg::ManualObject::make_unique(getParentObject()->getSceneManager());
		_material = dg::UnlitMaterial::make(getParentObject()->getSceneManager()->device());
		_material->texture = _texture.getTexture();
		_material->initialize(getParentObject()->getSceneManager()->device()); // TODO: this should be done automatically

		float m_focalLength = 858.5f;
		float width = data.width;
		float height = data.height;
		_size << width, height;

		float m_farPlane = 1.0;

	    //float dx = width/2.0  / m_focalLength * m_farPlane;
	    //float dy = height/2.0 / m_focalLength * m_farPlane;

	    _manualObject->clear();

	    DepthStencilStateDesc depthStencilDesc;

	    depthStencilDesc.DepthEnable = true;
	    if(getParentObject()->isOverlayMode()) // TODO: depth/stencil desc must become part of material
	    	depthStencilDesc.DepthEnable = false;

	    depthStencilDesc.StencilEnable = true;
	    depthStencilDesc.FrontFace.StencilPassOp = STENCIL_OP_REPLACE;
	    depthStencilDesc.FrontFace.StencilFunc = COMPARISON_FUNC_ALWAYS;
	    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;

	    _material->color = Color(1.0f,1.0f,1.0f,1.0f);

	    _manualObject->begin(_material, depthStencilDesc, dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	    _manualObject->position(0.0f,0.0f,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(0.0f,0.0f);

	    _manualObject->position(width,0.0f,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(1.0f,0.0f);

	    _manualObject->position(width,height,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(1.0f,1.0f);

	    _manualObject->index(0);
	    _manualObject->index(1);
	    _manualObject->index(2);

	    _manualObject->position(width,height,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(1.0f,1.0f);

	    _manualObject->position(0.0f, height,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(0.0f,1.0f);

	    _manualObject->position(0.0f,0.0f,0.0f);
	    _manualObject->color(dg::colors::White);
	    _manualObject->textureCoord(0.0f,0.0f);

	    _manualObject->index(3);
	    _manualObject->index(4);
	    _manualObject->index(5);

	    _manualObject->end();


	    _material_border = dg::UnlitMaterial::make(getParentObject()->getSceneManager()->device());

	    RenderTargetBlendDesc blend_desc;
		blend_desc.BlendEnable = true;
		blend_desc.RenderTargetWriteMask = COLOR_MASK_ALL;
		blend_desc.SrcBlend       = BLEND_FACTOR_SRC_ALPHA;
		blend_desc.DestBlend      = BLEND_FACTOR_ONE;
		blend_desc.BlendOp        = BLEND_OPERATION_ADD;
		blend_desc.SrcBlendAlpha  = BLEND_FACTOR_ZERO;
		blend_desc.DestBlendAlpha = BLEND_FACTOR_ONE;
		blend_desc.BlendOpAlpha   = BLEND_OPERATION_ADD;
		_material_border->setBlendDesc(blend_desc);




		float _border_spacing = 20.0f;
		float _width = 15.0f;
		float _length = 50.0f;

		float  d = _border_spacing;
		float  w = _width/2.0f;
		float  l = _length;

		//Color c0(0.0,0.0,0.0,0.0);
		Color c0(0.2,0.5,0.5,1.0);
		Color c1(0.2,0.5,0.5,1.0);

	    _manualObject->begin(_material_border, dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	    int voff;
	    voff += borderEdge(_manualObject.get(),  1.0f,  1.0,  0.0f,   0.0f, voff, d, w, l, c0, c1);
	    voff += borderEdge(_manualObject.get(), -1.0f,  1.0, width,   0.0f, voff, d, w, l, c0, c1);
	    voff += borderEdge(_manualObject.get(),  1.0f, -1.0,  0.0f, height, voff, d, w, l, c0, c1);
	    voff += borderEdge(_manualObject.get(), -1.0f, -1.0, width, height, voff, d, w, l, c0, c1);

	    _manualObject->end();

	}
}

void CanvasImageLayer::render()
{
	if(!_manualObject)
		return;

	if(getParentObject()->isOverlayMode())
		_material_border->opacity = 0.0f;

	getParentObject()->getSceneManager()->context()->SetStencilRef(getParentObject()->getStencilId());

	for(const auto& p : _manualObject->getSections())
		getParentObject()->getSceneManager()->render(p.get(), getParentObject()->getRenderMatrices());
}

void CanvasImageLayer::setOpacity(float opacity)
{
	if(_material)
	{
		_material->opacity = opacity;
		_material_border->opacity = opacity;
	}
}

}
