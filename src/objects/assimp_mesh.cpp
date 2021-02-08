
#include <dg/objects/assimp_mesh.hpp>
#include <dg/scene/scene_manager.hpp>
#include <dg/material/unlit_material.hpp>

#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace dg {

class AssimpMesh::Pimpl
{
public:
    Pimpl(AssimpMesh* q, SceneManager* manager, IMaterial::Ptr material);


    SceneManager* manager = nullptr;
    const aiScene* scene = nullptr;

    //std::map<std::string, OgreUtils::Material::SharedPtr> m_materials;

    dg::UnlitMaterial::Ptr default_material;
    IMaterial::Ptr material;

    AssimpMesh* q;

public:

    void loadMesh(const std::string& filename);
    void loadNode(aiNode* n, const aiMatrix4x4& parentTransform, bool skipNodesTransform);
    void loadSubMesh(const aiMesh* m, const aiMatrix4x4& transform);

    void setOpacity(float opacity);

};

AssimpMesh::Pimpl::Pimpl(AssimpMesh* q, SceneManager* manager, IMaterial::Ptr material) : q(q), manager(manager)
{
    default_material = dg::UnlitMaterial::make(manager->device());
    if(!material)
        this->material=default_material;
    else    
        this->material=material;
}

void AssimpMesh::Pimpl::loadMesh(const std::string& filename)
{

    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    scene = importer.ReadFile( filename,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_GenSmoothNormals       |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    // If the import failed, report it
    if(!scene)
      DG_THROW(importer.GetErrorString());

    /*
    Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(m_guid, RES_GROUP);

    m_aabb = Ogre::Aabb();

    assert(m_scene->mRootNode);
    loadNode(mesh, m_scene->mRootNode, aiMatrix4x4(), true); // skip roots transform

    mesh->_setBounds(m_aabb);

    Ogre::Vector3 s = m_aabb.getSize();
    mesh->_setBoundingSphereRadius(std::max(s.x, std::max(s.y, s.z))/2.0f);
    */

    loadNode(scene->mRootNode, aiMatrix4x4(), true); // skip roots transform
}

void AssimpMesh::Pimpl::loadNode(aiNode* n, const aiMatrix4x4& parentTransform, bool skipNodesTransform)
{
    aiMatrix4x4 transform;
    if(!skipNodesTransform)
        transform = parentTransform * n->mTransformation;
    else
        transform = parentTransform;


    for(unsigned int k = 0; k<n->mNumMeshes; ++k)
    {
        assert(n->mMeshes[k]<scene->mNumMeshes);
        const aiMesh* m = scene->mMeshes[n->mMeshes[k]];
        loadSubMesh(m, transform);
    }

    // process child nodes
    for(unsigned int i=0; i<n->mNumChildren; ++i)
        loadNode(n->mChildren[i], transform, false);
}


void AssimpMesh::Pimpl::loadSubMesh(const aiMesh* m, const aiMatrix4x4& transform)
{
    aiMatrix3x3 transform3(transform);

    q->begin(material, dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    bool use_normals = false;
    if(m->mNormals!=nullptr)
        use_normals = true;

    bool use_color = false;
    if(m->mColors[0])
        use_color = true;

    // fill in the vertex data
    for(size_t i=0; i<m->mNumVertices; ++i)
    {
        aiVector3D p = transform * m->mVertices[i];

        q->position(p.x,p.y,p.z);

        /*
        if(m->mTextureCoords[0]) {
            *vertexData++ = m->mTextureCoords[0][i].x;
            *vertexData++ = 1-m->mTextureCoords[0][i].y;
        }
        }*/

        if(use_normals)
        {
            aiVector3D n = transform3 * m->mNormals[i];
            n.Normalize();
            q->normal(n.x,n.y,n.z);
        }


        if(use_color)
        {
            if(m->mColors[0]) {
                q->color(Color(m->mColors[0][i].r,
                               m->mColors[0][i].g,
                               m->mColors[0][i].b,
                               m->mColors[0][i].a).toSRGB());
            }
            else
            {
                // TODO: use fixed color
            }
        }

        //TODO: m_aabb.merge(Ogre::Vector3(p.x,p.y,p.z));
    }


    for(unsigned int i=0; i<m->mNumFaces; ++i)
    {
        const aiFace& f = m->mFaces[i];

        if(f.mNumIndices==3) {
            for(unsigned int j=0; j<3; ++j)
            {
                //*indexPtr = f.mIndices[j];
                q->index(f.mIndices[j]);
            }
        }
        // else do nothing (only happens for lines that consists of 2 vertices only)
    }


/*
    // set the material for the submesh
    if(m_scene->HasMaterials()) {
        assert(m_scene->mMaterials!=NULL);
        assert(m->mMaterialIndex < m_scene->mNumMaterials);
        std::string materialName = m_guid + "_mat" + std::to_string(m->mMaterialIndex);
        createMaterial(m_guid + "_mat" + std::to_string(m->mMaterialIndex), m_scene->mMaterials[m->mMaterialIndex]);
        subMesh->setMaterialName(materialName);
    } else {
        std::string materialName = m_guid + "_defaultmat";
        createDefaultMaterial(materialName);
        subMesh->setMaterialName(materialName);
    }
*/

    q->end();
}

void AssimpMesh::Pimpl::setOpacity(float opacity)
{
    default_material->opacity = opacity;
}


AssimpMesh::AssimpMesh(SceneManager* manager, IMaterial::Ptr material) : ManualObject(manager)
{
    d.reset(new Pimpl(this, manager, material));
}

AssimpMesh::~AssimpMesh()
{

}

void AssimpMesh::load(const std::string& filename)
{
    clear();
    d->loadMesh(filename);
}

void AssimpMesh::setOpacity(float opacity)
{
    d->setOpacity(opacity);
}


}
