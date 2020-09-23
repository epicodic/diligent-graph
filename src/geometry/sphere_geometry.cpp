#include <dg/geometry/sphere_geometry.hpp>

namespace dg {

SphereGeometry::SphereGeometry(const Params& p, bool generate_normals, bool generate_uvs)
{
    int num = (p.heightSegments + 1) * (p.widthSegments + 1);

	_positions.resize(num);

	if(generate_normals)
		_normals.resize(num);

	if(generate_uvs)
		_uvs.resize(num);

	float phiLength = p.phiEnd - p.phiStart;
	float thetaLength = p.thetaEnd - p.thetaStart;

	int idx = 0;
	for (unsigned iy = 0; iy <= p.heightSegments; iy++)
	{

		float v = (float)iy / p.heightSegments;

		for (unsigned ix = 0; ix <= p.widthSegments; ix++, idx++ )
		{

			float u = (float)ix / p.widthSegments;
			float x = - p.radius * std::cos( p.phiStart   + u * phiLength ) * std::sin( p.thetaStart + v * thetaLength );
			float y =   p.radius * std::sin( p.phiStart   + u * phiLength ) * std::sin( p.thetaStart + v * thetaLength );
			float z =   p.radius * std::cos( p.thetaStart + v * thetaLength );

			_positions[idx] = {x, y, z};

			if(generate_normals)
				_normals[idx] = _positions[idx].normalized();

			if(generate_uvs)
				_uvs[idx] << u, v;

		}
	}

	// generate indices
	for ( int iy = 0; iy < p.heightSegments; iy++ )
	{

		for ( int ix = 0; ix < p.widthSegments; ix++ )
		{
			int width = p.widthSegments+1;


			int a = (iy  ) * width + (ix+1);
			int b = (iy  ) * width + (ix  );
			int c = (iy+1) * width + (ix  );
			int d = (iy+1) * width + (ix+1);

			if ( iy != 0 || p.thetaStart > 0 )
			{
				_indices.push_back(a);
				_indices.push_back(d);
				_indices.push_back(b);
			}

			if ( iy != p.heightSegments-1 || p.thetaEnd < M_PI )
			{
				_indices.push_back(b);
				_indices.push_back(d);
				_indices.push_back(c);
			}
		}
	}
}


}
