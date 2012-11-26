#include "include/fwconstant_def.h"
#include "include/vertex_process.h"
#include "include/shading_lib.h"

float4 eyepos : register(c0);
float4 lightPos : register(c1);
float4 lightAmb : register(c2);
float4 lightDif : register(c3);
float4 lightSpec : register(c4);

float4 matDif : register(c5);
float4 matSpec : register(c6);
float4 gloss_fs_fs_fs : register(c7);

vert2frag_General vs_main( app2vert IN )
{
	vert2frag_General OUT = (vert2frag_General)0;

	OUT = vs_output( IN );
	
	return OUT;
}

fragout ps_main( vert2frag_General IN )
{
	fragout OUT;
	float4 matColor = tex2D(DiffuseSampler, IN.texcoord.xy);
	float matCSpec = matColor.w;
	clip(matCSpec - eyepos.w);

	matColor = matColor * matColor;

	float3 normalDir = IN.normal.xyz;
	normalDir = normalize(normalDir);

	float3 viewWS = eyepos.xyz - IN.worldpos.xyz;

	viewWS = normalize(viewWS);

	float3 lightDirWS = lightPos.xyz - IN.worldpos.xyz;
	lightDirWS = normalize(lightDirWS);

	float dif = 0;
	float spec = 0;

	Blinn_BRDF( normalDir, lightDirWS, viewWS, gloss_fs_fs_fs.x, dif, spec );

	OUT.color = (lightAmb.xyzw * (normalDir.y * 0.4f + 0.6f) + dif * lightDif ) * matColor * matDif + matCSpec * matSpec * lightSpec * spec;	

	OUT.color = sqrt(OUT.color);

	OUT.color.a = 1;

	// linear z
	OUT.color1 = IN.hPos.z;

	return OUT;
}