
float GetFresnelOpt(float NdotI, float bias, float power)
{
	float facing = (1.0 - NdotI);
	return bias + pow(facing, power);  // opt: removed (1-bias) mul
}

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

void Skin_BRDF( in float3 normalDir, 
	in float3 lightDir,
	in float3 viewDir,
	in float power,
	inout float diffuse,
	inout float specular )
{
	// Half-LambertÂþ·´Éä
	float NdotL = dot(normalDir, lightDir);
	//diffuse = saturate( NdotL * 0.5f + 0.5f);
	//diffuse = saturate( NdotL );

	float halfLambert = (NdotL * 0.5) + 0.5;
	halfLambert = pow(halfLambert, 2.8);
	diffuse = halfLambert * (2 - 1 + NdotL); 


	// blinn BDRF
	float3 H = lightDir + viewDir;
	H = normalize(H);
	float HdotN = dot(H, normalDir);
	specular = pow( HdotN , power ) * diffuse;
}
