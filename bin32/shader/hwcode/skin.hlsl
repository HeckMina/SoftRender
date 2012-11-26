#include "include/fwconstant_def.h"
#include "include/vertex_process.h"
#include "include/shading_lib.h"

float4 eyePos : register(ps, c0);
float4 lightPos : register(ps, c1);
float4 lightAmb : register(ps, c2);
float4 lightDif : register(ps, c3);
float4 lightSpec : register(ps, c4);

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
	float matSpecColor = matColor.w;
	clip(matColor.w - eyePos.w);

	matColor = matColor * matColor;
	//matSpecColor = matSpecColor * matSpecColor * 2;

	float3 normalDir = IN.normal.xyz;
	normalDir = normalize(normalDir);

	float3 viewWS = eyePos.xyz - IN.worldpos.xyz;
	viewWS = normalize(viewWS);

	float3 lightDirWS = lightPos.xyz - IN.worldpos.xyz;
	lightDirWS = normalize(lightDirWS);

	float dif = 0;
	float spec = 0;

	Skin_BRDF( normalDir, lightDirWS, viewWS, gloss_fs_fs_fs.x, dif, spec );

	OUT.color = (lightAmb.xyzw * (normalDir.y * 0.4f + 0.6f) + dif * lightDif ) * matColor * matDif + matSpec * matSpecColor * lightSpec * spec;	

	float NdotE = saturate(dot(normalDir, viewWS));
	OUT.color += GetFresnelOpt(NdotE, 0, 5) * dif * 0.1;

	OUT.color = sqrt(OUT.color);

	OUT.color.a = 1;

	// linear z
	OUT.color1 = IN.hPos.z;

	return OUT;
}