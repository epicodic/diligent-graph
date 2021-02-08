#pragma once

#include <memory>

#include <dg/scene/raw_renderable.hpp>


// fwd decl
namespace Diligent { namespace GLTF {
class Model;
} }

namespace dg {

class GLTFMesh : public RawRenderable
{

public:
	DG_PTR(GLTFMesh)

	GLTFMesh();
	virtual ~GLTFMesh();

public:

	void load(const std::string& filename);
	void initialize(SceneManager* manager);

	void useLocalWorldFrame(bool use_local_frame=true);

	virtual void render(SceneManager* manager) override;
	GLTF::Model* getGLTFModel();


private:

	struct Pimpl;
	std::unique_ptr<Pimpl> d_;
};

}
