#pragma once

#include <vector>

#include <dg/core/fwds.hpp>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace dg {


class DynamicTexture
{

public:
	enum DataFormat
	{
		GREY8,
		GREY16,
		RGB8,
		RGBA8,
	};

	struct ImageData
	{
		const void* data;
		std::uint32_t stride;
		std::uint32_t width;
		std::uint32_t height;
		DataFormat format;
	};

	DynamicTexture() = default;

	bool update(IRenderDevice* device, IDeviceContext* context, const ImageData& data);

	/// Returns the underlying texture object. NOTE: only valid after first call to update() !
	RefCntAutoPtr<ITexture> getTexture() { return _texture; }

private:

	RefCntAutoPtr<ITexture> _texture;
	std::vector<char> _buffer; // temporary buffer
};


}
