#pragma once

#include <memory>

#include <dg/objects/manual_object.hpp>

namespace dg {

class AssimpMesh : public ManualObject
{

public:
    DG_PTR(AssimpMesh)

    AssimpMesh(SceneManager* manager, IMaterial::Ptr material = IMaterial::Ptr());
    virtual ~AssimpMesh();

public:

    void load(const std::string& filename);

    void setOpacity(float opacity);

private:

    class Pimpl;

    std::unique_ptr<Pimpl> d;
};

}
