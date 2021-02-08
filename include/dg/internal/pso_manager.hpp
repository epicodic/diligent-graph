#pragma once

#include <map>

#include <dg/core/fwds.hpp>


namespace dg {

class PSOManager
{
public:

	IPipelineState* getPSO(IRenderDevice* device, const PipelineStateDesc& desc);
	IPipelineState* getPSO(IRenderDevice* device, const PipelineStateDesc& desc, std::size_t desc_hash);

private:
	std::map<std::size_t, IPipelineState* > m_pso_pool_;
};

}
