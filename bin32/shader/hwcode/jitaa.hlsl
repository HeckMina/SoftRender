#include "include/postprocess.h"

float4 ps_main( vert2frag IN ) : COLOR0
{
	float4 OUT = 0;
	
	// test stuff
	float4 color0 = tex2D(tex0, IN.texcoord0.xy );
	float4 color1 = tex2D(tex1, IN.texcoord0.xy );

	return (color0 + color1) * 0.5;
}