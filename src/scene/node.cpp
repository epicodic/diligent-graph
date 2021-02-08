#include <dg/scene/node.hpp>

#include <dg/core/common.hpp>
#include <dg/scene/object.hpp>


namespace dg {

Node::~Node()
{
	if(parent_)
		parent_->removeChild(this);

	for(Object* obj : objects_)
		obj->node_ = nullptr;
}

Node::Ptr Node::createChild()
{
	Ptr child = std::make_shared<Node>();
	children_.push_back(child.get());
	child->parent_ = shared_from_this();
	child->child_idx_ = children_.size()-1;
	return child;
}


void Node::removeChild(const Node* child)
{
	if(child->parent_.get() != this)
		DG_THROW("Node was not a child of us");

	DG_ASSERT(_children.size() > child->_child_idx);

	std::size_t idx = child->child_idx_;
	if(children_.size()>1)
	{
		// remove child, by moving last child to its position (and updating the _child_idx of the moved child)
		children_[idx] = children_.back();
		children_[idx]->child_idx_ = idx;
	}

	// remove last item
	children_.pop_back();
}


Object* Node::attach(Object* obj)
{
	if(obj->node_.get() == this)
		return obj; // already attached

	if(obj->node_)
		obj->node_->detach(obj);

	obj->node_ = shared_from_this();
	objects_.push_back(obj);
	obj->object_idx_ = objects_.size()-1;

	obj->onAttached(this);

	return obj;
}

void Node::detach(Object* obj)
{
	if(obj->node_.get() != this)
		DG_THROW("Object was not attached to this node");

	DG_ASSERT(_objects.size() > obj->_object_idx);

	std::size_t idx = obj->object_idx_;
	if(objects_.size()>1)
	{
		// remove object, by moving last object to its position (and updating the _object_idx of the moved object)
		objects_[idx] = objects_.back();
		objects_[idx]->object_idx_ = idx;
	}

	obj->node_.reset();

	// remove last item
	objects_.pop_back();

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
	orientation_ = orientation_ * q;
}

void Node::lookAt(const Vector3& target, const Vector3& up)
{
	Vector3 zaxis = getPosition() - target;
	zaxis.normalize();

	Vector3 xaxis = up.cross(zaxis);
	xaxis.normalize();

	Vector3 yaxis = zaxis.cross(xaxis);
	yaxis.normalize();

	Matrix3 r;
	r.col(0) = xaxis;
	r.col(1) = yaxis;
	r.col(2) = zaxis;

	setOrientation(Quaternion(r));
}

}
