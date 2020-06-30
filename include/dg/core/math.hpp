#pragma once

#include <iostream>

#include <Eigen/Dense>

namespace dg {

using Real = double;

using Matrix3 = Eigen::Matrix<Real,3,3>;
using Matrix4 = Eigen::Matrix<Real,4,4>;

using Vector3 = Eigen::Matrix<Real,3,1>;
using Vector4 = Eigen::Matrix<Real,4,1>;

using Matrix3f = Eigen::Matrix<float,3,3>;
using Matrix4f = Eigen::Matrix<float,4,4>;

using Vector3f = Eigen::Matrix<float,3,1>;
using Vector4f = Eigen::Matrix<float,4,1>;



using Quaternion = Eigen::Quaternion<Real>;
using AngleAxis  = Eigen::AngleAxis<Real>;

using Quaternionf = Eigen::Quaternion<float>;
using AngleAxisf  = Eigen::AngleAxis<float>;

using Transform  = Matrix4;
using Transformf = Matrix4f;

namespace math {


inline Transform transformFromScaleRotTrans(const Vector3& scale, const Quaternion& q, const Vector3& t)
{
	// Ordering:
	//    1. Scale
	//    2. Rotate
	//    3. Translate

	Transform m;

	Matrix3 rot3x3 = q.toRotationMatrix();

	m(0,0) = scale.x() * rot3x3(0,0); m(0,1) = scale.y() * rot3x3(0,1); m(0,2) = scale.z() * rot3x3(0,2); m(0,3) = t.x();
	m(1,0) = scale.x() * rot3x3(1,0); m(1,1) = scale.y() * rot3x3(1,1); m(1,2) = scale.z() * rot3x3(1,2); m(1,3) = t.y();
	m(2,0) = scale.x() * rot3x3(2,0); m(2,1) = scale.y() * rot3x3(2,1); m(2,2) = scale.z() * rot3x3(2,2); m(2,3) = t.z();
	m(3,0) = 0; m(3,1) = 0; m(3,2) = 0; m(3,3) = 1;

	return m;
}

} }

