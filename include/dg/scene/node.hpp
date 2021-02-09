#pragma once

#include <vector>

#include <dg/core/common.hpp>

namespace dg {

class Object;
DG_DECL_PTR_FWD(Object)

class Node final : public std::enable_shared_from_this<Node>
{

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    DG_PTR(Node);

    Node() = default;
    ~Node();

    Ptr createChild();

    void removeChild(const Node::Ptr& child)
    {
        removeChild(child.get());
    }

    void removeChild(const Node* child);

    const std::vector<Node*>& getChildren() const { return children_; }
    std::vector<Node*>& getChildren() { return children_; }

    void setScale(const Vector3& scale)
    {
        scale_ = scale;
    }

    void setScale(Real scale)
    {
        setScale(Vector3(scale,scale,scale));
    }

    const Vector3& getScale() const
    {
        return scale_;
    }

    void setOrientation(const Quaternion& q)
    {
        orientation_ = q;
    }

    const Quaternion& getOrientation() const
    {
        return orientation_;
    }



    void rotateX(Real angle);
    void rotateY(Real angle);
    void rotateZ(Real angle);
    void rotate(const Quaternion& q);

    void lookAt(const Vector3& target, const Vector3& up = Vector3(0,1,0));

    void setPosition(const Vector3& position)
    {
        position_ = position;
    }

    const Vector3& getPosition() const
    {
        return position_;
    }


    const Transform& getTransform() const
    {
        return transform_;
    }

    const Transform& getDerivedTransform() const
    {
        return derivedTransform_;
    }

    void updateTransforms()
    {
        if(!isEnabled())
            return;

        transform_ = math::transformFromScaleRotTrans(scale_, orientation_, position_);

        if(!parent_)
            derivedTransform_ = transform_;
        else
            derivedTransform_ = parent_->derivedTransform_ * transform_;

        for(Node* child : children_)
            child->updateTransforms();
    }

    void setEnabled(bool enabled)
    {
        enabled_ = enabled;
    }

    bool isEnabled() const { return enabled_; }


    Object* attach(Object* obj);
    void detach(Object* obj);

    template <typename Obj>
    std::shared_ptr<Obj> attach(std::shared_ptr<Obj> obj)
    {
        attach(obj.get());
        return obj;
    }

    template <typename Obj>
    std::unique_ptr<Obj> attach(std::unique_ptr<Obj> obj)
    {
        attach(obj.get());
        return std::move(obj);
    }


/*
    struct unique_tag {};
    struct shared_tag {};
    template<typename T> struct tag_trait;
    template<typename T, typename D> struct tag_trait<std::unique_ptr<T,D>> { using tag = unique_tag; };
    template<typename T>             struct tag_trait<std::shared_ptr<T>>   { using tag = shared_tag; };

    ObjectPtr attach(std::shared_ptr<Object> obj, shared_tag) { attach(obj.get()); return obj; }
    ObjectUniquePtr attach(std::unique_ptr<Object> obj, unique_tag) { attach(obj.get()); return obj; }

public:
    template<typename T>
    void addObject(T&& obj) { addObject_internal(std::forward<T>(obj), typename tag_trait<std::remove_reference_t<T>>::tag{}); }

    ObjectPtr& attach(ObjectPtr& obj) { attach(obj.get()); return obj; }

    void detach(ObjectPtr obj)    { detach(obj.get()); }

    ObjectUniquePtr& attach(ObjectUniquePtr& obj)    {
        attach(obj.get());
        return obj;
    }

    void detach(ObjectUniquePtr obj) { detach(obj.get()); }
    */

    const std::vector<Object*>& getObjects() const { return objects_; }

private:

    bool enabled_ = true;
    Node::Ptr parent_;
    std::size_t child_idx_ = 0; // our index in _parent->children (for fast removal)
    std::vector<Node*> children_;
    std::vector<Object*> objects_;

    Vector3    position_    = Vector3(0.0,0.0,0.0);
    Quaternion orientation_ = Quaternion(1.0,0.0,0.0,0.0);
    Vector3    scale_       = Vector3(1.0,1.0,1.0);

    // both are computed from parent and _position, _orientation and _scale in updateTransforms()
    Transform  transform_;
    Transform  derivedTransform_;

};


}
