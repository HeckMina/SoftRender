#include "constant_def.h"

struct app2vert
{
	float4 pos : POSITION;
	float4 channel1 : TEXCOORD0;
	float4 channel2 : TEXCOORD1;
	float4 channel3 : TEXCOORD2;
};

struct vert2frag
{
	float4 pos : POSITION;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
	float4 texcoord2 : TEXCOORD2;
};

sampler tex0 : PS_REGSITER( s0 ) ;
sampler tex1 : PS_REGSITER( s1 ) ;
sampler tex2 : PS_REGSITER( s2 ) ;
sampler tex3 : PS_REGSITER( s3 ) ;
sampler tex4 : PS_REGSITER( s4 ) ;
sampler tex5 : PS_REGSITER( s5 ) ;
sampler tex6 : PS_REGSITER( s6 ) ;
sampler tex7 : PS_REGSITER( s7 ) ;

vert2frag vs_main( app2vert IN )
{
	vert2frag OUT = (vert2frag)0;

	// just transfer
	// in ws, no need transform, may optimize here

	OUT.pos = IN.pos;
	OUT.texcoord0 = IN.channel1;
	OUT.texcoord1 = IN.channel2;
	OUT.texcoord2 = IN.channel3;

	return OUT;
}