#pragma once

#include <dg/geometry/geometry.hpp>

namespace dg {

class BoxGeometry : public Geometry
{
public:

    struct Params
	{
        float width;
		float height;
		float depth;
		int widthSegments;
		int heightSegments;
		int depthSegments;

		explicit Params(float width = 1.0, float height = 1.0, float depth = 1.0,
                        int widthSegments = 1, int heightSegments = 1, int depthSegments = 1) :
            width(width), height(height), depth(depth),
			widthSegments(widthSegments),
			heightSegments(heightSegments),
            depthSegments(depthSegments)
		{}
	};

	explicit BoxGeometry(const Params& params = Params());

private:

	void buildPlane(int u, int v, int w, float udir, float vdir, float width, float height, float depth, int gridX, int gridY);
};
    
}
