#pragma once

#include <dg/core/common.hpp>
#include <dg/core/type_id.hpp>

#include <dg/scene/node.hpp>

namespace dg {

/**
 * Base class for all objects that can be attached to nodes.
 */
class Object
{
public:

    DG_PTR(Object)

    Object(TypeId type_id) : type_id_(type_id) {}
    virtual ~Object();

    TypeId typeId() { return type_id_; }

    template <typename T>
    T* cast() {
        if(type_id<T>() == type_id_)
            return reinterpret_cast<T*>(this);
        else
            return nullptr;
    }

    template <typename T>
    const T* cast() const {
        if(type_id<T>() == type_id_)
            return reinterpret_cast<const T*>(this);
        else
            return nullptr;
    }

public:

    /// Returns the node, this object is attached to
    Node::ConstPtr getNode() const { return node_; }
    const Node::Ptr& getNode() { return node_; }

protected:

    /// Called when object is attached to a node
    virtual void onAttached(Node* node) {}

    /// Called when object is detached from node
    virtual void onDetached(Node* node) {}

private:

    friend class Node;

    Node::Ptr node_;
    std::size_t object_idx_ = 0; // our index in _node->_objects (managed by Node)
    TypeId type_id_;
};


}
