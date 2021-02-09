#pragma once

#include <dg/core/frustum.hpp>

#include <dg/scene/object.hpp>


namespace dg {


class Camera : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Camera() : Object(type_id<Camera>()) {}

    void setFov(float fov) {
        frustum_.fov = fov;
        proj_matrix_dirty_ = true;
    }

    void setNearPlane(float near) {
        frustum_.near_plane = near;
        proj_matrix_dirty_ = true;
    }

    void setFarPlane(float far) {
        frustum_.far_plane = far;
        proj_matrix_dirty_ = true;
    }

    void setAspect(float aspect) {
        frustum_.aspect = aspect;
        proj_matrix_dirty_ = true;
    }

    void setPrincipal(const Eigen::Vector2f& principal) {
        frustum_.principal = principal;
        proj_matrix_dirty_ = true;
    }

    const Frustum& getFrustum() const
    {
        return frustum_;
    }

public:

    const Matrix4f& getProjectionMatrix() const {
        if(proj_matrix_dirty_)
            proj_matrix_ = frustum_.computeProjectionMatrix();
        return proj_matrix_;
    }

private:

    Frustum frustum_;

    bool proj_matrix_dirty_ = true;
    mutable Matrix4f proj_matrix_;


};


}
