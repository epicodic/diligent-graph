#pragma once

#include <vector>

#include <dg/core/common.hpp>

namespace dg {

class IGeometry
{
public:

	virtual ~IGeometry() = default;

	virtual const std::vector<Vector3>& getPositions() const = 0;
	virtual const std::vector<Vector3>& getNormals() const = 0;
	virtual const std::vector<Vector2f>& getUVs() const = 0;
	virtual const std::vector<std::uint32_t>& getIndices() const = 0;

};


class Geometry : public IGeometry
{
public:

	virtual const std::vector<Vector3>& getPositions() const override
	{
		return _positions;
	}

	virtual const std::vector<Vector3>& getNormals() const override
	{
		return _normals;
	}

	virtual const std::vector<Vector2f>& getUVs() const override
	{
		return _uvs;
	}

	virtual const std::vector<std::uint32_t>& getIndices() const override
	{
		return _indices;
	}

	std::vector<Vector3>& getPositions()
	{
		return _positions;
	}

	std::vector<Vector3>& getNormals()
		{
		return _normals;
	}

	std::vector<Vector2f>& getUVs()
	{
		return _uvs;
	}


protected:

	std::vector<Vector3> _positions;
	std::vector<Vector3> _normals;
	std::vector<Vector2f> _uvs;
	std::vector<std::uint32_t> _indices;

};

}
