#pragma once

#include <dg/core/common.hpp>

namespace dg {


#define DG_COMMON_CONSTANTS_VS \
	float4x4 g_worldViewProj;  \
    float4x4 g_worldView;      \
    float4x4 g_view;           \

#define DG_COMMON_CONSTANTS_VS_CODE              \
	"cbuffer CommonConstantsVS"                  \
	"{"                                          \
		DG_STRINGIFY(DG_COMMON_CONSTANTS_VS)     \
	"};"

struct CommonConstantsVS
{
	DG_COMMON_CONSTANTS_VS
};


}

