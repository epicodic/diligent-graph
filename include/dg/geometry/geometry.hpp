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
        return positions_;
    }

    virtual const std::vector<Vector3>& getNormals() const override
    {
        return normals_;
    }

    virtual const std::vector<Vector2f>& getUVs() const override
    {
        return uvs_;
    }

    virtual const std::vector<std::uint32_t>& getIndices() const override
    {
        return indices_;
    }

    std::vector<Vector3>& getPositions()
    {
        return positions_;
    }

    std::vector<Vector3>& getNormals()
        {
        return normals_;
    }

    std::vector<Vector2f>& getUVs()
    {
        return uvs_;
    }


protected:

    std::vector<Vector3> positions_;
    std::vector<Vector3> normals_;
    std::vector<Vector2f> uvs_;
    std::vector<std::uint32_t> indices_;

};

}
