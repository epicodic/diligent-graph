
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
	auto it = m_pso_pool.find(desc_hash);
	if(it!=m_pso_pool.end())
		return it->second;
	else {
		IPipelineState* pso = nullptr;
		PipelineStateCreateInfo createInfo;
		createInfo.PSODesc = desc;
		device->CreatePipelineState(createInfo, &pso);
		m_pso_pool[desc_hash] = pso;
		return pso;
	}
}

}
