
#include <dg/internal/pso_manager.hpp>
#include <dg/internal/pso_hash.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>

#include <iostream>
namespace dg {

IPipelineState* PSOManager::getPSO(IRenderDevice* device, const PipelineStateDesc& desc)
{
	return getPSO(device, desc, hash_value(desc));
}


IPipelineState* PSOManager::getPSO(IRenderDevice* device, const PipelineStateDesc& desc, std::size_t desc_hash)
{
	auto it = m_pso_pool_.find(desc_hash);
	if(it!=m_pso_pool_.end())
		return it->second;
	else {
		IPipelineState* pso = nullptr;
		PipelineStateCreateInfo create_info;
		create_info.PSODesc = desc;
		device->CreatePipelineState(create_info, &pso);
		m_pso_pool_[desc_hash] = pso;
		return pso;
	}
}

}
