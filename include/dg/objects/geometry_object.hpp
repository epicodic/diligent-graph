#pragma once

#include <memory>

#include <dg/scene/renderable.hpp>

#include <dg/geometry/geometry.hpp>

namespace dg {

class GeometryObject : public Renderable
{

public:

	GeometryObject(SceneManager* manager, const IGeometry& geometry, const std::vector<Color>& colors = std::vector<Color>());

public:

	void setMaterial(IMaterial::Ptr m);
	IMaterial::ConstPtr getMaterial() const { return _material; }

};

}
