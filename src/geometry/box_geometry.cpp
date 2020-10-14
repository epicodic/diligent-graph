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

    int idx0 = _positions.size();

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
            _positions.push_back(vector);

            vector[ u ] = 0;
            vector[ v ] = 0;
            vector[ w ] = depth > 0 ? 1 : - 1;
            _normals.push_back(vector);

            // uvs
            _uvs.emplace_back(float(ix)/float(grid_x), 1.0f-(float(iy)/float(grid_y)) );
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
            _indices.push_back(a);
            _indices.push_back(b);
            _indices.push_back(d);

            _indices.push_back(b);
            _indices.push_back(c);
            _indices.push_back(d);
        }
    }
}


BoxGeometry::BoxGeometry(const Params& p)
{
    _positions.clear();
	_normals.clear();
	_uvs.clear();

    buildPlane( 2, 1, 0, -1, -1, p.depth, p.height,  p.width,  p.depthSegments, p.heightSegments); // px
    buildPlane( 2, 1, 0,  1, -1, p.depth, p.height, -p.width,  p.depthSegments, p.heightSegments); // nx
    buildPlane( 0, 2, 1,  1,  1, p.width, p.depth,   p.height, p.widthSegments, p.depthSegments); // py
    buildPlane( 0, 2, 1,  1, -1, p.width, p.depth,  -p.height, p.widthSegments, p.depthSegments); // ny
    buildPlane( 0, 1, 2,  1, -1, p.width, p.height,  p.depth,  p.widthSegments, p.heightSegments); // pz
    buildPlane( 0, 1, 2, -1, -1, p.width, p.height, -p.depth,  p.widthSegments, p.heightSegments); // nz

}

}
