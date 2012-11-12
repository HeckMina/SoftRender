struct app2vert
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct vert2frag
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD0;
	float3 worldpos : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
};

float4x4 mvp;
float4x4 world;

vert2frag main( app2vert IN )
{
	vert2frag OUT = (vert2frag)0;

	OUT.pos = mul(mvp , IN.pos);
	OUT.normal = mul((float3x3)world , IN.normal);
	OUT.worldpos = mul(world , IN.pos.xyz);
	OUT.texcoord = IN.texcoord;

	return OUT;
}