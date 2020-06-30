#include <dg/scene/node.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/object.hpp>


namespace dg {

Node::~Node()
{
	if(_parent)
		_parent->removeChild(this);

	for(Object* obj : _objects)
		obj->_node = nullptr;
}

Node::Ptr Node::createChild()
{
	Ptr child = std::make_shared<Node>();
	_children.push_back(child.get());
	child->_parent = shared_from_this();
	child->_child_idx = _children.size()-1;
	return child;
}


void Node::removeChild(const Node* child)
{
	if(child->_parent.get() != this)
		DG_THROW("Node was not a child of us");

	DG_ASSERT(_children.size() > child->_child_idx);

	std::size_t idx = child->_child_idx;
	if(_children.size()>1)
	{
		// remove child, by moving last child to its position (and updating the _child_idx of the moved child)
		_children[idx] = _children.back();
		_children[idx]->_child_idx = idx;
	}

	// remove last item
	_children.pop_back();
}


Object* Node::attach(Object* obj)
{
	if(obj->_node.get() == this)
		return obj; // already attached

	if(obj->_node)
		obj->_node->detach(obj);

	obj->_node = shared_from_this();
	_objects.push_back(obj);
	obj->_object_idx = _objects.size()-1;

	obj->onAttached(this);

	return obj;
}

void Node::detach(Object* obj)
{
	if(obj->_node.get() != this)
		DG_THROW("Object was not attached to this node");

	DG_ASSERT(_objects.size() > obj->_object_idx);

	std::size_t idx = obj->_object_idx;
	if(_objects.size()>1)
	{
		// remove object, by moving last object to its position (and updating the _object_idx of the moved object)
		_objects[idx] = _objects.back();
		_objects[idx]->_object_idx = idx;
	}

	obj->_node.reset();

	// remove last item
	_objects.pop_back();

	obj->onDetached(this);
}



void Node::rotateX(Real angle)
{
	rotate(Quaternion(AngleAxis(angle, Vector3::UnitX())));
}

void Node::rotateY(Real angle)
{
	rotate(Quaternion(AngleAxis(angle, Vector3::UnitY())));
}

void Node::rotateZ(Real angle)
{
	rotate(Quaternion(AngleAxis(angle, Vector3::UnitZ())));
}

void Node::rotate(const Quaternion& q)
{
	_orientation = _orientation * q;
}

void Node::lookAt(const Vector3& target, const Vector3& up)
{
	Vector3 zaxis = getPosition() - target;
	zaxis.normalize();

	Vector3 xaxis = up.cross(zaxis);
	xaxis.normalize();

	Vector3 yaxis = zaxis.cross(xaxis);
	yaxis.normalize();

	Matrix3 R;
	R.col(0) = xaxis;
	R.col(1) = yaxis;
	R.col(2) = zaxis;

	setOrientation(Quaternion(R));
}

}
