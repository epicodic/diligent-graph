#include <dg/geometry/sphere_geometry.hpp>

namespace dg {

SphereGeometry::SphereGeometry(const Params& p, bool generate_normals, bool generate_uvs)
{
    int num = (p.height_segments + 1) * (p.width_segments + 1);

    positions_.resize(num);

    if(generate_normals)
        normals_.resize(num);

    if(generate_uvs)
        uvs_.resize(num);

    float phi_length = p.phi_end - p.phi_start;
    float theta_length = p.theta_end - p.theta_start;

    int idx = 0;
    for (unsigned iy = 0; iy <= p.height_segments; iy++)
    {

        float v = (float)iy / p.height_segments;

        for (unsigned ix = 0; ix <= p.width_segments; ix++, idx++ )
        {

            float u = (float)ix / p.width_segments;
            float x = - p.radius * std::cos( p.phi_start   + u * phi_length ) * std::sin( p.theta_start + v * theta_length );
            float y =   p.radius * std::sin( p.phi_start   + u * phi_length ) * std::sin( p.theta_start + v * theta_length );
            float z =   p.radius * std::cos( p.theta_start + v * theta_length );

            positions_[idx] = {x, y, z};

            if(generate_normals)
                normals_[idx] = positions_[idx].normalized();

            if(generate_uvs)
                uvs_[idx] << u, v;

        }
    }

    // generate indices
    for ( int iy = 0; iy < p.height_segments; iy++ )
    {

        for ( int ix = 0; ix < p.width_segments; ix++ )
        {
            int width = p.width_segments+1;


            int a = (iy  ) * width + (ix+1);
            int b = (iy  ) * width + (ix  );
            int c = (iy+1) * width + (ix  );
            int d = (iy+1) * width + (ix+1);

            if ( iy != 0 || p.theta_start > 0 )
            {
                indices_.push_back(a);
                indices_.push_back(d);
                indices_.push_back(b);
            }

            if ( iy != p.height_segments-1 || p.theta_end < M_PI )
            {
                indices_.push_back(b);
                indices_.push_back(d);
                indices_.push_back(c);
            }
        }
    }
}


}
