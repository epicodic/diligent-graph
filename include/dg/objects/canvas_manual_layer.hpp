#pragma once

#include "canvas_object.hpp"

#include <dg/material/unlit_material.hpp>
#include <dg/material/dynamic_texture.hpp>
#include <dg/objects/manual_object.hpp>

namespace dg {


class CanvasManualLayer : public CanvasLayer
{
public:
    CanvasManualLayer(CanvasObject* parent_object);

    virtual void render() override;
    virtual void setOpacity(float opacity) override;

    dg::ManualObject* manualObject() { return manualObject_.get(); }
    const dg::ManualObject* manualObject() const { return manualObject_.get(); }

private:

    dg::ManualObject::UniquePtr manualObject_;
};

}
