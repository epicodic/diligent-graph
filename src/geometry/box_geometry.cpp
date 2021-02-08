#include <dg/geometry/box_geometry.hpp>

namespace dg {


void BoxGeometry::buildPlane(int u, int v, int w, float udir, float vdir, float width, float height, float depth, int grid_x, int grid_y) 
{
    const float segment_width = width / float(grid_x);
    const float segment_height = height / float(grid_y);

    const float width_half = width / 2;
    const float height_half = height / 2;
    const float depth_half = depth / 2;

    const int grid_x1 = grid_x + 1;
    const int grid_y1 = grid_y + 1;


    Vector3 vector;

    int idx0 = positions_.size();

    // generate vertices, normals and uvs
    for ( int iy = 0; iy < grid_y1; iy++ ) 
    {
        float y = iy * segment_height - height_half;

        for ( int ix = 0; ix < grid_x1; ix ++ ) 
        {
            float x = ix * segment_width - width_half;

            vector[ u ] = x * udir;
            vector[ v ] = y * vdir;
            vector[ w ] = depth_half;
            positions_.push_back(vector);

            vector[ u ] = 0;
            vector[ v ] = 0;
            vector[ w ] = depth > 0 ? 1 : - 1;
            normals_.push_back(vector);

            // uvs
            uvs_.emplace_back(float(ix)/float(grid_x), 1.0f-(float(iy)/float(grid_y)) );
        }
    }

    // indices
    // 1. you need three indices to draw a single face
    // 2. a single segment consists of two faces
    // 3. so we need to generate six (2*3) indices per segment
    for (int iy=0; iy<grid_y; iy++) 
    {
        for (int ix=0; ix<grid_x; ix++) 
        {
            int a = idx0 + ix + grid_x1 * iy;
            int b = idx0 + ix + grid_x1 * ( iy + 1 );
            int c = idx0 + ( ix + 1 ) + grid_x1 * ( iy + 1 );
            int d = idx0 + ( ix + 1 ) + grid_x1 * iy;

            // faces
            indices_.push_back(a);
            indices_.push_back(b);
            indices_.push_back(d);

            indices_.push_back(b);
            indices_.push_back(c);
            indices_.push_back(d);
        }
    }
}


BoxGeometry::BoxGeometry(const Params& p)
{
    positions_.clear();
    normals_.clear();
    uvs_.clear();

    buildPlane( 2, 1, 0, -1, -1, p.depth, p.height,  p.width,  p.depth_segments, p.height_segments); // px
    buildPlane( 2, 1, 0,  1, -1, p.depth, p.height, -p.width,  p.depth_segments, p.height_segments); // nx
    buildPlane( 0, 2, 1,  1,  1, p.width, p.depth,   p.height, p.width_segments, p.depth_segments); // py
    buildPlane( 0, 2, 1,  1, -1, p.width, p.depth,  -p.height, p.width_segments, p.depth_segments); // ny
    buildPlane( 0, 1, 2,  1, -1, p.width, p.height,  p.depth,  p.width_segments, p.height_segments); // pz
    buildPlane( 0, 1, 2, -1, -1, p.width, p.height, -p.depth,  p.width_segments, p.height_segments); // nz

}

}
