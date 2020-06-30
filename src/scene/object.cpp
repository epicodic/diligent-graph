#include <dg/scene/object.hpp>

#include <dg/scene/node.hpp>

namespace dg {

Object::~Object()
{
	if(_node)
		_node->detach(this);
}

}
