#include <dg/scene/object.hpp>

#include <dg/scene/node.hpp>

namespace dg {

Object::~Object()
{
    if(node_)
        node_->detach(this);
}

}
