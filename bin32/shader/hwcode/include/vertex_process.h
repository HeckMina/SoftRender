
void skinPositionNormal( float3 skinPos, float3 skinNorm, float blendWeightsArray[4], int indexArray[4], out float3 position, out float3 normal )
{
	// calculate the pos/normal using the "normal" weights 
	// and accumulate the weights to calculate the last weight
	position = 0.0f;
	normal = 0.0f; 

	float lastWeight = 0.0f;
	for (int iBone = 0; iBone < 3; iBone++)
	{
		lastWeight = lastWeight + blendWeightsArray[iBone];

		position += mul( g_mLocalMatrixArray[indexArray[iBone]], float4(skinPos,1) ) * blendWeightsArray[iBone];
		normal += mul((float3x3)g_mLocalMatrixArray[indexArray[iBone]], skinNorm) * blendWeightsArray[iBone];
	}
	lastWeight =  1.0f - lastWeight; 

	// Now that we have the calculated weight, add in the final influence
	position += (mul( g_mLocalMatrixArray[indexArray[3]], float4(skinPos,1) ) * lastWeight);
	normal += (mul((float3x3) g_mLocalMatrixArray[indexArray[3]], skinNorm) * lastWeight);
}


#if defined( SKINED )

struct app2vert
{
	float4 pos : POSITION;
	float4 normal : TEXCOORD0;
	float2 texcoord : TEXCOORD1;
	float4 inBlendWeights : TEXCOORD2;
	int4 inBlendIndices : TEXCOORD3;
};

#else

struct app2vert
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD0;
	float2 texcoord : TEXCOORD1;
};

#endif

struct vert2frag_General
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD0;
	float3 worldpos : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
	float4 hPos : TEXCOORD3;
};

vert2frag_General vs_output( app2vert IN )
{
	vert2frag_General OUT = (vert2frag_General)0;

	float3 position;
	float3 normal;

#if defined( SKINED )
	float blendWeightsArray[4] = (float[4])IN.inBlendWeights;    
	int   indexArray[4] = (int[4])IN.inBlendIndices;    
	skinPositionNormal( IN.pos.xyz, IN.normal.xyz, blendWeightsArray, indexArray, position, normal );
#else
	position = IN.pos.xyz;
	normal = IN.normal.xyz;
#endif

	OUT.pos = mul(mvp , float4(position, 1));
	OUT.normal = mul((float3x3)world , normal);
	OUT.worldpos = mul(world , position);
	OUT.texcoord = IN.texcoord;
	OUT.hPos = OUT.pos;
	OUT.hPos.z 
	return OUT;
}