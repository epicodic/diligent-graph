#pragma once

#include <vector>

#include <dg/core/fwds.hpp>
#include <dg/scene/object.hpp>
#include <dg/scene/render_order.hpp>
#include <dg/scene/scene_manager.hpp>

namespace dg {

class SceneManager;
class RawRenderable : public Object
{

public:

    RawRenderable() : Object(type_id<RawRenderable>()) {}

    virtual ~RawRenderable() = default;

public:

    void setRenderOrder(RenderOrder order)
    {
        render_order_ = order;
    }

    virtual void render(SceneManager* manager) = 0;

protected:

    friend class SceneManager;

    RenderOrder  render_order_ ;

};

}
