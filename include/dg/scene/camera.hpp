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
		_frustum.fov = fov;
		_proj_matrix_dirty = true;
	}

	void setNearPlane(float near) {
		_frustum.near_plane = near;
		_proj_matrix_dirty = true;
	}

	void setFarPlane(float far) {
		_frustum.far_plane = far;
		_proj_matrix_dirty = true;
	}

	void setAspect(float aspect) {
		_frustum.aspect = aspect;
		_proj_matrix_dirty = true;
	}

	void setPrincipal(const Eigen::Vector2f& principal) {
		_frustum.principal = principal;
		_proj_matrix_dirty = true;
	}

	const Frustum& getFrustum() const
	{
		return _frustum;
	}

public:

	const Matrix4f& getProjectionMatrix() const {
		if(_proj_matrix_dirty)
			_proj_matrix = _frustum.computeProjectionMatrix();
		return _proj_matrix;
	}

private:

	Frustum _frustum;

	bool _proj_matrix_dirty = true;
	mutable Matrix4f _proj_matrix;


};


}
