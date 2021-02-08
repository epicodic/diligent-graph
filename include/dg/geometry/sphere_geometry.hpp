#pragma once

#include <cmath>

#include <dg/geometry/geometry.hpp>

namespace dg {

class SphereGeometry : public Geometry
{
public:

    struct Params
	{
		float radius;
		int width_segments, height_segments;
		float phi_start, phi_end;
		float theta_start, theta_end;

		explicit Params(float radius = 1.0f,
			   int widthSegments = 48, int heightSegments = 32,
			   float phiStart = 0.0f, float phiEnd = 2.0f*M_PI,
			   float thetaStart = 0.0f, float thetaEnd = M_PI) :
			radius(radius),
			width_segments(widthSegments),
			height_segments(heightSegments),
			phi_start(phiStart), phi_end(phiEnd),
			theta_start(thetaStart), theta_end(thetaEnd)
		{}
	};

	explicit SphereGeometry(const Params& params = Params(), bool generate_normals = true, bool generate_uvs = true);

};
    
}
