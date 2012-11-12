struct vert2frag
{
	float4 pos : POSITION;
	float4 texcoord0 : TEXCOORD0;
	float4 texcoord1 : TEXCOORD1;
	float3 texcoord2 : TEXCOORD2;
};

// sampler0


// sampler1

sampler tex0 : register(s0);
sampler tex1 : register(s1);

float4 main( vert2frag IN ) : COLOR0
{
	float4 OUT = 0;
	
	// test stuff
	float4 color0 = tex2D(tex0, IN.texcoord0.xy );
	float4 color1 = tex2D(tex1, IN.texcoord0.xy );

	return (color0 + color1) * 0.5;
}