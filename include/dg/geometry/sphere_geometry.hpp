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
		int widthSegments, heightSegments;
		float phiStart, phiEnd;
		float thetaStart, thetaEnd;

		explicit Params(float radius = 1.0f,
			   int widthSegments = 48, int heightSegments = 32,
			   float phiStart = 0.0f, float phiEnd = 2.0f*M_PI,
			   float thetaStart = 0.0f, float thetaEnd = M_PI) :
			radius(radius),
			widthSegments(widthSegments),
			heightSegments(heightSegments),
			phiStart(phiStart), phiEnd(phiEnd),
			thetaStart(thetaStart), thetaEnd(thetaEnd)
		{}
	};

	explicit SphereGeometry(const Params& params = Params(), bool generate_normals = true, bool generate_uvs = true);

};
    
}
