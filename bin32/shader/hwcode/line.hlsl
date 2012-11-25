#include "include/fwconstant_def.h"
#include "include/shading_lib.h"

struct app2vert
{
	float4 pos : POSITION;
	float4 color : TEXCOORD0;
};

struct vert2frag
{
	float4 pos : POSITION;
	float4 color : COLOR;
};

vert2frag vs_main( app2vert IN )
{
	vert2frag OUT = (vert2frag)0;

	OUT.pos = mul(mvp , float4(IN.pos.xyz, 1));
	OUT.color = IN.color;
	return OUT;
}

fragout ps_main( vert2frag IN )
{
	fragout OUT;

	OUT.color = 1;
	OUT.color.a = 1;
	OUT.color1 = 0;

	return OUT;
}