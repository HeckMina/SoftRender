#include "include/postprocess.h"

float4 PI_psOffsets[8] : PS_REGSITER(c0); 
float4 psWeights[8] : PS_REGSITER(c8);

float4 ps_main( vert2frag IN ) : COLOR0
{
	float4 OUT = 0;
	
	float4 sum = 0;

	float4 col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[0].xy);
	sum += col * psWeights[0].x;  

	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[1].xy);
	sum += col * psWeights[1].x;  
	
	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[2].xy);
	sum += col * psWeights[2].x;  

	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[3].xy);
	sum += col * psWeights[3].x;

	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[4].xy);
	sum += col * psWeights[4].x;  
	
	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[5].xy);
	sum += col * psWeights[5].x;  
	
	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[6].xy);
	sum += col * psWeights[6].x;  
	
	col = tex2D(tex0, IN.texcoord0.xy + PI_psOffsets[7].xy);
	sum += col * psWeights[7].x;
	
	OUT = sum;
  
  return OUT;
}