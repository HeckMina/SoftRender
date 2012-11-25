#include "include/postprocess.h"

float4 dofParam : PS_REGSITER(c0);

float4 ps_main( vert2frag IN ) : COLOR0
{
	float4 OUT = 0;
	
	// test stuff
	float4 colorBlur = tex2D(tex0, IN.texcoord0.xy );
	float depth = tex2D(tex1, IN.texcoord0.xy ).x;

	OUT = colorBlur + 0.15 *  float4(0.5, 0.69, 0.84, 1);
	OUT.a = saturate(abs(depth - 0.03) * 30);
	
	return OUT;
}