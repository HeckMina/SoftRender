float4 eyePos_test : register(c0);
float4 lightPos : register(c1);
float4 lightAmb : register(c2);
float4 lightDif : register(c3);
float4 lightSpec : register(c4);

float4 matDif : register(c5);
float4 matSpec : register(c6);
float4 gloss_fsB_fsP_fsS : register(c7);

void Blinn_BRDF( in float3 normalDir, 
in float3 lightDir,
in float3 viewDir,
in float power,
inout float diffuse,
inout float specular )
{
	// Half-LambertÂþ·´Éä
	float NdotL = dot(normalDir, lightDir);
	diffuse = saturate( NdotL );

	// blinn BDRF
	float3 H = lightDir + viewDir;
	H = normalize(H);
	float HdotN = dot(H, normalDir);
	specular = pow( HdotN , power ) * diffuse;
}

struct vert2frag
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD0;
	float3 worldpos : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
};

sampler DiffuseSampler : register(s0) = sampler_state
{
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler NormalSampler : register(s1) = sampler_state
{
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

float4 main( vert2frag IN ) : COLOR0
{
	float4 OUT;
	float4 matColor = tex2D(DiffuseSampler, IN.texcoord.xy);
	float matCSpec = matColor.w;
	clip(matCSpec - eyePos_test.w);

	matColor = matColor * matColor;

	float3 normalDir = IN.normal.xyz;
	normalDir = normalize(normalDir);

	float3 viewWS = eyePos_test.xyz - IN.worldpos.xyz;
	viewWS = normalize(viewWS);

	float3 lightDirWS = lightPos.xyz - IN.worldpos.xyz;
	lightDirWS = normalize(lightDirWS);

	float dif = 0;
	float spec = 0;

	Blinn_BRDF( normalDir, lightDirWS, viewWS, gloss_fsB_fsP_fsS.x, dif, spec );

	OUT = (lightAmb.xyzw * (normalDir.y * 0.4f + 0.6f) + dif * lightDif ) * matColor * matDif + matCSpec * matSpec * lightSpec * spec;	

	OUT = sqrt(OUT);

	OUT.a = 1;

	return OUT;
}