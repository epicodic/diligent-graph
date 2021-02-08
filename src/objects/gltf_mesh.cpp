
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
	std::unique_ptr<GLTF_PBR_Renderer>    renderer;
	std::unique_ptr<GLTF::Model>          model;

	RefCntAutoPtr<IBuffer>                camera_attribs_cb;
	RefCntAutoPtr<IBuffer>                light_attribs_cb;
	RefCntAutoPtr<ITextureView>           env_map_srv;
	RefCntAutoPtr<IBuffer>                env_map_attribs_cb;

    bool initialized = false;
    bool use_local_frame = true;

	std::string filename;
};

struct EnvMapRenderAttribs
{
    ToneMappingAttribs tm_attribs;

    float average_log_lum;
    float mip_level;
    float unusued1;
    float unusued2;
};


GLTFMesh::GLTFMesh()
{
	d.reset(new Pimpl);
}

GLTFMesh::~GLTFMesh() = default;

void GLTFMesh::load(const std::string& filename)
{
    d->filename = filename;
    d->initialized = false;
}

void GLTFMesh::initialize(SceneManager* manager)
{
	RefCntAutoPtr<ITexture> env_map = manager->getEnvironmentMap();

    if(env_map)
        d->env_map_srv = env_map->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    GLTF_PBR_Renderer::CreateInfo renderer_ci;
    renderer_ci.RTVFmt         = manager->swapChain()->GetDesc().ColorBufferFormat;
    renderer_ci.DSVFmt         = manager->swapChain()->GetDesc().DepthBufferFormat;
    renderer_ci.AllowDebugView = false;
    renderer_ci.UseIBL         = true;
    renderer_ci.FrontCCW       = true;
    d->renderer.reset(new GLTF_PBR_Renderer(manager->device(), manager->context(), renderer_ci));


    CreateUniformBuffer(manager->device(), sizeof(CameraAttribs), "Camera attribs buffer", &d->camera_attribs_cb);
    CreateUniformBuffer(manager->device(), sizeof(LightAttribs), "Light attribs buffer", &d->light_attribs_cb);
    CreateUniformBuffer(manager->device(), sizeof(EnvMapRenderAttribs), "Env map render attribs buffer", &d->env_map_attribs_cb);

    StateTransitionDesc barriers [] =
    {
        {d->camera_attribs_cb,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {d->light_attribs_cb,   RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {d->env_map_attribs_cb, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true}//,
        //{env_map,                RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
    };
    manager->context()->TransitionResourceStates(_countof(barriers), barriers);

    if(d->env_map_srv)
        d->renderer->PrecomputeCubemaps(manager->device(), manager->context(), d->env_map_srv);

    d->model.reset(new GLTF::Model(manager->device(), manager->context(), d->filename));

	d->renderer->InitializeResourceBindings(*d->model, d->camera_attribs_cb, d->light_attribs_cb);

	d->initialized = true;
}

void GLTFMesh::useLocalWorldFrame(bool use_local_frame) 
{
	d->use_local_frame = use_local_frame;
}

void GLTFMesh::render(SceneManager* manager)
{
	if(!d->initialized)
		initialize(manager);


	// TODO get light info from scene manager
    float3 light_direction = float3(-1,-1,-1);
    float4 light_color     = float4(1, 1, 1, 1);
    float  light_intensity = 5.f;


    Vector3 camera_local;
    Matrix4 local_T_model;
    Matrix4 view_proj_local;
    
    if(d->use_local_frame)
    {
        Matrix4 view_T_world = manager->getView();
        Matrix4 inv_view_T_world = view_T_world.inverse();
        Matrix4 world_T_model = getNode()->getDerivedTransform();
        Matrix4 world_T_local = Matrix4::Identity();
        world_T_local.block<3,1>(0,3) = inv_view_T_world.block<3,1>(0,3);

        Matrix4 view_T_local = view_T_world * world_T_local;
        Matrix4 inv_view_T_local = view_T_local.inverse();

        local_T_model = world_T_local.inverse()*world_T_model;
        camera_local  = inv_view_T_local.block<3,1>(0,3);
        view_proj_local = manager->getProj()*view_T_local;
    }
    else
    {
        local_T_model = getNode()->getDerivedTransform();
        camera_local = manager->getCameraWorldPosition();
        view_proj_local = manager->getViewProj();
    }

    { // map helper scope
        MapHelper<CameraAttribs> cam_attribs(manager->context(), d->camera_attribs_cb, MAP_WRITE, MAP_FLAG_DISCARD);

        matrix_to_float4x4(manager->getProj(), cam_attribs->mProjT);
        matrix_to_float4x4(view_proj_local, cam_attribs->mViewProjT);
        
        Matrix4 view_proj_inv =  view_proj_local.inverse();
        matrix_to_float4x4(view_proj_inv, cam_attribs->mViewProjInvT);

        float3 p;
        vector_to_float3(camera_local, p);
        cam_attribs->f4Position    = float4(p, 1);
    }

    { // map helper scope
        MapHelper<LightAttribs> light_attribs(manager->context(), d->light_attribs_cb, MAP_WRITE, MAP_FLAG_DISCARD);
        light_attribs->f4Direction = light_direction;
        light_attribs->f4Intensity = light_color * light_intensity;
    }

    GLTF_PBR_Renderer::RenderInfo render_params;

    matrix_to_float4x4t(local_T_model, render_params.ModelTransform);

    render_params.IBLScale = 0.5;
    render_params.OcclusionStrength = 1.0;
    d->renderer->Render(manager->context(), *d->model, render_params);
}

GLTF::Model* GLTFMesh::getGLTFModel()
{
	return d->model.get();
}


}
