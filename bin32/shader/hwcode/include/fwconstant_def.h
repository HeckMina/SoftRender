#include "constant_def.h"

float4x4 mvp : VS_REGSITER( c0 );
float4x4 world : VS_REGSITER( c4 );
#define HKG_MAX_BLEND_MATRICES 54
float4x4 g_mLocalMatrixArray[HKG_MAX_BLEND_MATRICES] : VS_REGSITER( c8 );

sampler DiffuseSampler : PS_REGSITER( s0 ) = sampler_state
{
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler NormalSampler : PS_REGSITER( s1 ) = sampler_state
{
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler SpecularSampler : PS_REGSITER( s2 ) = sampler_state
{
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

struct fragout
{
	float4 color : COLOR0;
	float4 color1 : COLOR1;
};
