#pragma once

#include "canvas_object.hpp"

#include <dg/material/unlit_material.hpp>
#include <dg/material/dynamic_texture.hpp>
#include <dg/objects/manual_object.hpp>

namespace dg {


class CanvasImageLayer : public CanvasLayer
{
public:
	CanvasImageLayer(CanvasObject* parent_object);

    void setScale(float scale);
    float getScale() const;
	void update(const dg::DynamicTexture::ImageData& data);

	virtual void render() override;
	virtual void setOpacity(float opacity) override;
	virtual Eigen::Vector2f getSize() const override { return _size; }

private:

    float _scale = 1.0f;

    dg::ManualObject::UniquePtr _manualObject;
    dg::UnlitMaterial::Ptr _material;
    dg::UnlitMaterial::Ptr _material_border;
    dg::DynamicTexture _texture;

    Eigen::Vector2f _size = Eigen::Vector2f(0.0f,0.0f);

};



}
