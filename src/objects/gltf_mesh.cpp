
#include <dg/objects/gltf_mesh.hpp>

#include <dg/core/conversion.hpp>

#include <DiligentCore/Common/interface/BasicMath.hpp>

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
//#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>
#include <DiligentCore/Graphics/GraphicsTools/interface/GraphicsUtilities.h>
#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>

#include <DiligentTools/AssetLoader/interface/GLTFLoader.hpp>
#include <DiligentFX/GLTF_PBR_Renderer/interface/GLTF_PBR_Renderer.hpp>


namespace Diligent
{
#include "Shaders/Common/public/BasicStructures.fxh"
#include "Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh"
}

namespace dg {


struct GLTFMesh::Pimpl
{
	std::unique_ptr<GLTF_PBR_Renderer>    _renderer;
	std::unique_ptr<GLTF::Model>          _model;

	RefCntAutoPtr<IBuffer>                _camera_attribs_cb;
	RefCntAutoPtr<IBuffer>                _light_attribs_cb;
	RefCntAutoPtr<ITextureView>           _env_map_srv;
	RefCntAutoPtr<IBuffer>                _env_map_attribs_cb;


	bool initialized = false;

	std::string filename;
};

struct EnvMapRenderAttribs
{
    ToneMappingAttribs TMAttribs;

    float AverageLogLum;
    float MipLevel;
    float Unusued1;
    float Unusued2;
};


GLTFMesh::GLTFMesh()
{
	d.reset(new Pimpl);
}

GLTFMesh::~GLTFMesh()
{

}

void GLTFMesh::load(const std::string& filename)
{
    d->filename = filename;
    d->initialized = false;
}

void GLTFMesh::initialize(SceneManager* manager)
{
	RefCntAutoPtr<ITexture> env_map = manager->getEnvironmentMap();

    if(env_map)
        d->_env_map_srv = env_map->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    GLTF_PBR_Renderer::CreateInfo RendererCI;
    RendererCI.RTVFmt         = manager->swapChain()->GetDesc().ColorBufferFormat;
    RendererCI.DSVFmt         = manager->swapChain()->GetDesc().DepthBufferFormat;
    RendererCI.AllowDebugView = false;
    RendererCI.UseIBL         = true;
    RendererCI.FrontCCW       = true;
    d->_renderer.reset(new GLTF_PBR_Renderer(manager->device(), manager->context(), RendererCI));


    CreateUniformBuffer(manager->device(), sizeof(CameraAttribs), "Camera attribs buffer", &d->_camera_attribs_cb);
    CreateUniformBuffer(manager->device(), sizeof(LightAttribs), "Light attribs buffer", &d->_light_attribs_cb);
    CreateUniformBuffer(manager->device(), sizeof(EnvMapRenderAttribs), "Env map render attribs buffer", &d->_env_map_attribs_cb);

    StateTransitionDesc Barriers [] =
    {
        {d->_camera_attribs_cb,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {d->_light_attribs_cb,   RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {d->_env_map_attribs_cb, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true}//,
        //{env_map,                RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
    };
    manager->context()->TransitionResourceStates(_countof(Barriers), Barriers);

    if(d->_env_map_srv)
        d->_renderer->PrecomputeCubemaps(manager->device(), manager->context(), d->_env_map_srv);

    d->_model.reset(new GLTF::Model(manager->device(), manager->context(), d->filename));

	d->_renderer->InitializeResourceBindings(*d->_model, d->_camera_attribs_cb, d->_light_attribs_cb);

	d->initialized = true;
}

void GLTFMesh::render(SceneManager* manager)
{
	if(!d->initialized)
		initialize(manager);


	// TODO get light info from scene manager
    float3 m_LightDirection = float3(-1,-1,-1);
	//Vector3 m_LightDirection(1,1,1);
    float4 m_LightColor     = float4(1, 1, 1, 1);
    float  m_LightIntensity = 5.f;


    {
        MapHelper<CameraAttribs> camAttribs(manager->context(), d->_camera_attribs_cb, MAP_WRITE, MAP_FLAG_DISCARD);

        matrix_to_float4x4(manager->getProj(), camAttribs->mProjT);
        matrix_to_float4x4(manager->getViewProj(), camAttribs->mViewProjT);
        //matrix_to_float4x4(manager->getViewProjInv(), camAttribs->mViewProjInvT);
        Matrix4 viewProjInv =  manager->getViewProj().inverse();
        matrix_to_float4x4(viewProjInv, camAttribs->mViewProjInvT);

        float3 p;
        vector_to_float3(manager->getCameraWorldPosition(), p);
        camAttribs->f4Position    = float4(p, 1);
    }

    {
        MapHelper<LightAttribs> lightAttribs(manager->context(), d->_light_attribs_cb, MAP_WRITE, MAP_FLAG_DISCARD);
        lightAttribs->f4Direction = m_LightDirection;
        lightAttribs->f4Intensity = m_LightColor * m_LightIntensity;
    }

    GLTF_PBR_Renderer::RenderInfo m_RenderParams;

    matrix_to_float4x4t(getNode()->getDerivedTransform(), m_RenderParams.ModelTransform);
    //m_RenderParams.ModelTransform = float4x4::Identity(); // m_ModelTransform * m_ModelRotation.ToMatrix();
    m_RenderParams.IBLScale = 0.5;
    m_RenderParams.OcclusionStrength = 1.0;
    d->_renderer->Render(manager->context(), *d->_model, m_RenderParams);

}

GLTF::Model* GLTFMesh::getGLTFModel()
{
	return d->_model.get();
}


}
