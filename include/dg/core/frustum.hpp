#pragma once

#include <cmath>

#include <dg/core/math.hpp>

namespace dg {


class Frustum
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	float fov = M_PI / 4.0f;
	float near_plane = 1.0f;
	float far_plane = 100.0f;
	float aspect = 16.0f/9.0f;

	Eigen::Vector2f principal = Eigen::Vector2f(0.5f,0.5f);

public:

	Eigen::Matrix4f computeProjectionMatrix() const;

};


}
