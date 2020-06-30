#pragma once

#include <memory>

#include <dg/scene/renderable.hpp>

namespace dg {

class GeometryObject : public Renderable
{

public:

	GeometryObject(SceneManager* manager);
	virtual ~GeometryObject();

public:

	void setMaterial(IMaterial::Ptr m);
	IMaterial::ConstPtr getMaterial() const { return _material; }

protected:

	std::vector<Vector3> _positions;
	std::vector<Vector3> _normals;
	std::vector<std::uint32_t> _indices;

protected:

	void generate();

private:

	SceneManager* _manager;

};

}
