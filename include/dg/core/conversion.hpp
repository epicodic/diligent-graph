#pragma once

#include "math.hpp"

#include <DiligentCore/Common/interface/BasicMath.hpp>

namespace dg {
using namespace Diligent;

template <typename T>
inline void matrix_to_float3x3(const Eigen::Matrix<T,3,3>& s, float4x4& d) // NOLINT
{
	d._11 = s(0,0);
	d._12 = s(0,1);
	d._13 = s(0,2);

	d._21 = s(1,0);
	d._22 = s(1,1);
	d._23 = s(1,2);

	d._31 = s(2,0);
	d._32 = s(2,1);
	d._33 = s(2,2);
}

template <typename T>
inline void matrix_to_float3x3t(const Eigen::Matrix<T,3,3>& s, float3x3& d) // NOLINT
{
	d._11 = s(0,0);
	d._12 = s(1,0);
	d._13 = s(2,0);

	d._21 = s(0,1);
	d._22 = s(1,1);
	d._23 = s(2,1);

	d._31 = s(0,2);
	d._32 = s(1,2);
	d._33 = s(2,2);
}

template <typename T>
inline void matrix_to_float4x4(const Eigen::Matrix<T,4,4>& s, float4x4& d) // NOLINT
{
	d._11 = s(0,0);
	d._12 = s(0,1);
	d._13 = s(0,2);
	d._14 = s(0,3);

	d._21 = s(1,0);
	d._22 = s(1,1);
	d._23 = s(1,2);
	d._24 = s(1,3);

	d._31 = s(2,0);
	d._32 = s(2,1);
	d._33 = s(2,2);
	d._34 = s(2,3);

	d._41 = s(3,0);
	d._42 = s(3,1);
	d._43 = s(3,2);
	d._44 = s(3,3);
}

template <typename T>
inline void matrix_to_float4x4t(const Eigen::Matrix<T,4,4>& s, float4x4& d) // NOLINT
{
	d._11 = s(0,0);
	d._12 = s(1,0);
	d._13 = s(2,0);
	d._14 = s(3,0);

	d._21 = s(0,1);
	d._22 = s(1,1);
	d._23 = s(2,1);
	d._24 = s(3,1);

	d._31 = s(0,2);
	d._32 = s(1,2);
	d._33 = s(2,2);
	d._34 = s(3,2);

	d._41 = s(0,3);
	d._42 = s(1,3);
	d._43 = s(2,3);
	d._44 = s(3,3);
}

template <typename T>
inline void vector_to_float3(const Eigen::Matrix<T,3,1>& s, float3& d) // NOLINT
{
	d.x = s(0);
	d.y = s(1);
	d.z = s(2);
}

template <typename T>
inline void vector_to_float4(const Eigen::Matrix<T,4,1>& s, float4& d) // NOLINT
{
	d.x = s(0);
	d.y = s(1);
	d.z = s(2);
	d.w = s(3);
}

}
