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
	float4 worldpos_tx : TEXCOORD0;
	float4 normal_ty : TEXCOORD1;
	float4 tangent : TEXCOORD2;
};

vert2frag main( app2vert IN )
{
	vert2frag OUT = (vert2frag)0;

	// just transfer
	// in ws, no need transform, may optimize here

	OUT.pos = IN.pos;
	OUT.worldpos_tx = IN.channel1;
	OUT.normal_ty = IN.channel2;
	OUT.tangent = IN.channel3;

	return OUT;
}