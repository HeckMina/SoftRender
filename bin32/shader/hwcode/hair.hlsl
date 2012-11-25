#include "include/fwconstant_def.h"
#include "include/shading_lib.h"

float4 eyepos	: VS_REGSITER( c8 )
				: PS_REGSITER( c0 );
float4 lightPos : register(c1);
float4 lightAmb : register(c2);
float4 lightDif : register(c3);
float4 lightSpec : register(c4);

float4 matDif : register(c5);
float4 matSpec : register(c6);
float4 gloss_fs_fs_fs : register(c7);




struct app2vert
{
	float4 pos_tx : POSITION;
	float4 prevpos_ty : TEXCOORD0;
	float4 tangent_width : TEXCOORD1;
};

struct vert2frag
{
	float4 pos : POSITION;
	float4 worldpos_tx : TEXCOORD0;
	float4 normal_ty : TEXCOORD1;
	float4 tangent : TEXCOORD2;
	float4 hPos : TEXCOORD3;
};

vert2frag vs_main( app2vert IN )
{
	vert2frag OUT = (vert2frag)0;

	// 世界矩阵是IDENTITY
	OUT.pos = mul(mvp, float4(IN.pos_tx.xyz, 1.f));

	OUT.worldpos_tx.xyz = IN.pos_tx.xyz;// + world[3].xyz;

	// tangent in WS calc
	OUT.tangent.xyz = IN.prevpos_ty.xyz - OUT.worldpos_tx.xyz;
	OUT.tangent.xyz = normalize(OUT.tangent.xyz);

	float3 tangentInProoj = mul((float3x3)mvp, OUT.tangent.xyz);
	tangentInProoj = normalize(tangentInProoj);

	// normal in WS calc
	OUT.normal_ty.xyz = normalize(cross(IN.tangent_width.xyz, OUT.tangent.xyz));

	// dot angle
	float3 viewWS = eyepos.xyz - OUT.worldpos_tx.xyz;
	viewWS.y = 0;

	viewWS = normalize(viewWS);

	//float3 flatView = viewWS;
	//flatView.y = 0;
	//flatView = normalize(flatView);
	//float3 flatNormal = OUT.normal_ty.xyz;
	//flatNormal.y = 0;
	//flatNormal = normalize(flatNormal);

	//float NdotE = 1.0 - dot(flatView, flatNormal);
	//NdotE = 0.0f + 1.0 * pow(NdotE, 4.0);
	// width recalc
	//float width = IN.tangent_width.w * NdotE;

	float width = IN.tangent_width.w * 2.f;

	// now, flatten
	float flatDir = (IN.pos_tx.w - 0.5) * 2;

	// determing flatten on x or y
	float DotX = dot(tangentInProoj, float3(1,0,0));
	//DotX = pow(DotX, 0.1);
	DotX = abs(DotX);
	//if(DotX < 0.7)
	//OUT.pos.x += flatDir * width;// * (1 - DotX);
	//else
	//OUT.pos.y += flatDir * width;// * DotX;

	// flatten along the other axis
	OUT.pos.x += flatDir * width * tangentInProoj.y;
	OUT.pos.y -= flatDir * width * tangentInProoj.x;

	// texcoord
	OUT.worldpos_tx.w = IN.pos_tx.w;
	OUT.normal_ty.w = IN.prevpos_ty.w;

	OUT.hPos = OUT.pos;

	return OUT;
}

void Half_Lambert( in float3 normalDir, 
in float3 lightDir,
inout float diffuse
)
{
	// Half-Lambert漫反射
	float NdotL = dot(normalDir, lightDir);
	diffuse = saturate( NdotL * 0.5f + 0.5f);
}

void Kajiya_Kay_Specular( in float3 tangentDir, 
in float3 lightDir,
in float3 viewDir,
in float power,
in float diffuse,
inout float specular )
{
	// blinn BDRF
	float3 H = lightDir + viewDir;
	H = normalize(H);
	float HdotT = dot(H, tangentDir);
	specular = (pow(  sqrt( 1.f - HdotT * HdotT), power )) * diffuse;
}

float3 ShiftTangent(float3 T, float3 N, float shift  )
{
	float3 shiftT = T + N * shift;
	return normalize(shiftT);
}

fragout ps_main( vert2frag IN )
{
	fragout OUT;
	float4 matColor = tex2D(DiffuseSampler, float2(IN.worldpos_tx.w, IN.normal_ty.w));
	float4 matSpec = matColor;

	// forward clip
	clip(matColor.w - eyepos.w);

	// backward clip
	//clip( lightPos.w - matColor.w);

	matColor = matColor * matColor;

	float3 normalDir = IN.normal_ty.xyz;
	normalDir = normalize(normalDir);

	float3 viewWS = eyepos.xyz - IN.worldpos_tx.xyz;
	viewWS = normalize(viewWS);

	float3 lightDirWS = lightPos.xyz - IN.worldpos_tx.xyz;
	lightDirWS = normalize(lightDirWS);

	float3 tangentWS = IN.tangent;
	tangentWS = normalize(tangentWS);

	float dif = 0;
	float spec = 0;

	//Kajiya_Kay_BRDF( normalDir, tangentWS, lightDirWS, viewWS, 100.f, dif, spec );
	Half_Lambert( normalDir, lightDirWS, dif );
	float spec1;
	Kajiya_Kay_Specular(  ShiftTangent(IN.tangent, normalDir, 0.1f), lightDirWS, viewWS, 500.f, dif, spec1 );
	spec += spec1;
	Kajiya_Kay_Specular(  ShiftTangent(IN.tangent, normalDir, -0.1f), lightDirWS, viewWS, 40.f, dif, spec1 );
	spec += spec1 * 0.3;

	OUT.color = matColor * matDif * ( lightDif * dif + lightAmb.xyzw * (normalDir.y * 0.4f + 0.6f)) + spec * lightSpec * matSpec;	

	OUT.color = sqrt(OUT.color);
	OUT.color.a = matSpec.w;

	float depth = IN.hPos.z / 1000.0;
	if( matColor.w < 0.3)
		depth = 1;
	OUT.color1 = depth;

	return OUT;
}