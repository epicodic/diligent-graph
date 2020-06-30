#pragma once

#include <dg/core/hash.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>

namespace dg {



template<>
struct hash<ShaderResourceVariableDesc> {
std::size_t operator()(const ShaderResourceVariableDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.ShaderStages);
	hash_combine(seed, v.Type);
	return seed;
}
};

template<>
struct hash<StaticSamplerDesc> {
std::size_t operator()(const StaticSamplerDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.ShaderStages);
    hash_combine(seed, v.Desc);
	return seed;
}
};

template<>
struct hash<SamplerDesc> {
std::size_t operator()(const SamplerDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.MinFilter);
    hash_combine(seed, v.MagFilter);
    hash_combine(seed, v.MipFilter);
    hash_combine(seed, v.AddressU);
    hash_combine(seed, v.AddressV);
    hash_combine(seed, v.AddressW);
    hash_combine(seed, v.MipLODBias);
    hash_combine(seed, v.MaxAnisotropy);
    hash_combine(seed, v.ComparisonFunc);
    hash_combine(seed, v.BorderColor);
    hash_combine(seed, v.MinLOD);
    hash_combine(seed, v.MaxLOD);
	return seed;
}
};

template<>
struct hash<PipelineResourceLayoutDesc> {
std::size_t operator()(const PipelineResourceLayoutDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.DefaultVariableType);
    hash_combine(seed, v.NumVariables);
    for(int i=0; i<v.NumVariables; ++i)
    	hash_combine(seed, v.Variables[i]);
    hash_combine(seed, v.NumStaticSamplers);
    for(int i=0; i<v.NumStaticSamplers; ++i)
    	hash_combine(seed, v.StaticSamplers[i]);
	return seed;
}
};

template<>
struct hash<BlendStateDesc> {
std::size_t operator()(const BlendStateDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.AlphaToCoverageEnable);
    hash_combine(seed, v.IndependentBlendEnable);
	hash_combine(seed, v.RenderTargets);
	return seed;
}
};

template<>
struct hash<RenderTargetBlendDesc> {
std::size_t operator()(const RenderTargetBlendDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.BlendEnable);
    hash_combine(seed, v.LogicOperationEnable);
	hash_combine(seed, v.SrcBlend);
	hash_combine(seed, v.DestBlend);
	hash_combine(seed, v.BlendOp);
	hash_combine(seed, v.SrcBlendAlpha);
	hash_combine(seed, v.DestBlendAlpha);
	hash_combine(seed, v.BlendOpAlpha);
	hash_combine(seed, v.LogicOp);
	hash_combine(seed, v.RenderTargetWriteMask);
	return seed;
}
};

template<>
struct hash<RasterizerStateDesc> {
std::size_t operator()(const RasterizerStateDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.FillMode);
    hash_combine(seed, v.CullMode);
	hash_combine(seed, v.FrontCounterClockwise);
	hash_combine(seed, v.DepthClipEnable);
	hash_combine(seed, v.ScissorEnable);
	hash_combine(seed, v.AntialiasedLineEnable);
	hash_combine(seed, v.DepthBias);
	hash_combine(seed, v.DepthBiasClamp);
	hash_combine(seed, v.SlopeScaledDepthBias);
	return seed;
}
};

template<>
struct hash<DepthStencilStateDesc> {
std::size_t operator()(const DepthStencilStateDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.DepthEnable);
    hash_combine(seed, v.DepthWriteEnable);
	hash_combine(seed, v.DepthFunc);
	hash_combine(seed, v.StencilEnable);
	hash_combine(seed, v.StencilReadMask);
	hash_combine(seed, v.StencilWriteMask);
	hash_combine(seed, v.FrontFace);
	hash_combine(seed, v.BackFace);
	return seed;
}
};

template<>
struct hash<StencilOpDesc> {
std::size_t operator()(const StencilOpDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.StencilFailOp);
    hash_combine(seed, v.StencilDepthFailOp);
	hash_combine(seed, v.StencilPassOp);
	hash_combine(seed, v.StencilFunc);
	return seed;
}
};

template<>
struct hash<InputLayoutDesc> {
std::size_t operator()(const InputLayoutDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.NumElements);
    for(auto i=0;  i<v.NumElements; ++i)
    	hash_combine(seed, v.LayoutElements[i]);
	return seed;
}
};

template<>
struct hash<LayoutElement> {
std::size_t operator()(const LayoutElement& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.InputIndex);
    hash_combine(seed, v.BufferSlot);
    hash_combine(seed, v.NumComponents);
    hash_combine(seed, v.ValueType);
    hash_combine(seed, v.IsNormalized);
    hash_combine(seed, v.RelativeOffset);
    hash_combine(seed, v.Stride);
    hash_combine(seed, v.Frequency);
    hash_combine(seed, v.InstanceDataStepRate);
	return seed;
}
};

template<>
struct hash<GraphicsPipelineDesc> {
std::size_t operator()(const GraphicsPipelineDesc& v) const
{
	std::size_t seed = 0;
	hash_combine(seed, v.pVS);
	hash_combine(seed, v.pPS);
	hash_combine(seed, v.pDS);
	hash_combine(seed, v.pHS);
	hash_combine(seed, v.pGS);
	hash_combine(seed, v.BlendDesc);
	hash_combine(seed, v.SampleMask);
	hash_combine(seed, v.RasterizerDesc);
	hash_combine(seed, v.DepthStencilDesc);
	hash_combine(seed, v.InputLayout);
	hash_combine(seed, v.PrimitiveTopology);
	hash_combine(seed, v.NumViewports);
	hash_combine(seed, v.NumRenderTargets);
	hash_combine(seed, v.RTVFormats);
	hash_combine(seed, v.DSVFormat);
	hash_combine(seed, v.SmplDesc);
	hash_combine(seed, v.NodeMask);
	return seed;
}
};

template<>
struct hash<SampleDesc> {
std::size_t operator()(const SampleDesc& v) const
{
	std::size_t seed = 0;
	hash_combine(seed, v.Count);
	hash_combine(seed, v.Quality);
	return seed;
}
};

template<>
struct hash<ComputePipelineDesc> {
std::size_t operator()(const ComputePipelineDesc& v) const
{
	std::size_t seed = 0;
	hash_combine(seed, v.pCS);
	return seed;
}
};

template<>
struct hash<PipelineStateDesc> {
std::size_t operator()(const PipelineStateDesc& v) const
{
    std::size_t seed = 0;
    hash_combine(seed, v.IsComputePipeline);
    hash_combine(seed, v.SRBAllocationGranularity);
    hash_combine(seed, v.CommandQueueMask);
    hash_combine(seed, v.ResourceLayout);

    if(!v.IsComputePipeline)
    	hash_combine(seed, v.GraphicsPipeline);
    else
    	hash_combine(seed, v.ComputePipeline);
    return seed;
}
};


}
