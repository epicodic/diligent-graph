#include <dg/core/frustum.hpp>

namespace dg {

constexpr float INFINITE_FAR_PLANE_ADJUST = 0.00001f;

Eigen::Matrix4f Frustum::computeProjectionMatrix() const
{

    float theta_y (fov * 0.5f);
    float tan_theta_y = std::tan(theta_y);
    float tan_theta_x = tan_theta_y * aspect;

    float half_w = tan_theta_x * near_plane;
    float half_h = tan_theta_y * near_plane;

    float left   = -principal.x() * 2.0f * half_w;
    float right  = left + 2.0f * half_w;
    float bottom = -principal.y() * 2.0f * half_h;
    float top    = bottom + 2.0f * half_h;

    float inv_w = 1 / (right - left);
    float inv_h = 1 / (top - bottom);
    float inv_d = 1 / (far_plane - near_plane);

    // Calc matrix elements
    float A = 2 * near_plane * inv_w;
    float B = 2 * near_plane * inv_h;
    float C = (right + left) * inv_w;
    float D = (top + bottom) * inv_h;
    float q, qn;

    if (far_plane == 0)
    {
        // Infinite far plane
        q = INFINITE_FAR_PLANE_ADJUST - 1.0f;
        qn = near_plane * (INFINITE_FAR_PLANE_ADJUST - 2.0f);
    }
    else
    {
        q = - (far_plane + near_plane) * inv_d;
        qn = -2 * (far_plane * near_plane) * inv_d;
    }

    Eigen::Matrix4f proj_mat;

    proj_mat = Eigen::Matrix4f::Zero();
    proj_mat(0,0) = A;
    proj_mat(0,2) = C;
    proj_mat(1,1) = B;
    proj_mat(1,2) = D;
    proj_mat(2,2) = q;
    proj_mat(2,3) = qn;
    proj_mat(3,2) = -1;

    /*
     A  0  C 0
     0  B  D 0
     0  0  q qn
     0  0 -1 0

     */

    return proj_mat;
}

}
