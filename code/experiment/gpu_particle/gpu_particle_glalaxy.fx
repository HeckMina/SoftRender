//--------------------------------------------------------------------------------------
// File: SimpleSample.fx
//
// The effect file for the SimpleSample sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
//dcl_2d s0

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
float3 g_LightDir;                  // Light's direction in world space
float4 g_LightDiffuse;              // Light's diffuse color
texture g_MeshTexture;              // Color texture for mesh
float4 g_bbRight;
float4 g_bbUp;

float    g_fTime;                   // App's time in seconds
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix



//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler: register(s1) = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler VTFSampler: register(s0);

sampler Prev_ParticleUpdateSampler: register(s0) = 
	sampler_state
{
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler Curr_ParticleUpdateSampler: register(s1) = 
	sampler_state
{
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler tex0: register(s0) = 
	sampler_state
{
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler tex1: register(s1) = 
	sampler_state
{
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

VS_OUTPUT RenderParticleVS( float4 vPos : POSITION,
                         float2 vTexCoord0 : TEXCOORD0,
						 float2 vTexCoord1 : TEXCOORD1)
{
    VS_OUTPUT Output;
    
    // Transform the position from object space to homogeneous projection space

	float4 posVTF = 1;
	posVTF = tex2Dlod(VTFSampler, float4(vTexCoord1,1,1));

	float2 offset = vTexCoord0 - float2(0.5, 0.5);
	posVTF.xyz += g_bbRight.xyz * (offset.x * 0.1 * posVTF.w);
	posVTF.xyz += g_bbUp.xyz * (offset.y * 0.1 * posVTF.w);

	Output.Position = mul(float4(posVTF.xyz, 1), g_mWorldViewProjection);



    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;

    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = float4(1,0,0,1);

    return Output;
}

PS_OUTPUT RenderParticlePS( VS_OUTPUT In ) 
{ 
	PS_OUTPUT Output;

	// Lookup mesh texture and modulate it with diffuse
	Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV);
	clip( Output.RGBColor.a - 0.01 );

	return Output;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}

technique RenderParticle
{
    pass P0
    {          
		//Zenable = false;
		//Cullmode = none;
        VertexShader = compile vs_3_0 RenderParticleVS();
        PixelShader  = compile ps_3_0 RenderParticlePS(); 
    }
}


float4 g_staticForce;
float4 g_timeVar;

float4 g_attract0;
float4 g_attract1;
float4 g_attract2;

float3 Verlet(in float3 prev, in float3 curr, in float3 force)
{
	return curr + 0.9f * (curr - prev) + force * g_timeVar.x * g_timeVar.x;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT Verlet_StaticForcePS( float2 TextureUV : TEXCOORD0 ) 
{ 
	PS_OUTPUT Output = (PS_OUTPUT)0;

	// Lookup mesh texture and modulate it with diffuse
	float3 prevPos = tex2D(Prev_ParticleUpdateSampler, TextureUV);
	float3 currPos = tex2D(Curr_ParticleUpdateSampler, TextureUV);

	float3 vec0 = g_attract0.xyz - currPos;
	float3 vec1 = g_attract1.xyz - currPos;
	float3 vec2 = g_attract2.xyz - currPos;

	vec0 = normalize(vec0) / length(vec0);
	vec1 = normalize(vec1) / length(vec1);
	vec2 = normalize(vec2) / length(vec2);

	float3 mergeForce = vec0 + vec1 + vec2;
	mergeForce = normalize(mergeForce);

	float3 nextPos = Verlet(prevPos, currPos, mergeForce);

	Output.RGBColor.xyz = nextPos;

	return Output;
}

PS_OUTPUT Eular_AttractionPS( float2 TextureUV : TEXCOORD0 ) 
{ 
	PS_OUTPUT Output = (PS_OUTPUT)0;

	// Lookup mesh texture and modulate it with diffuse
	float3 prevVel = tex2D(tex0, TextureUV);
	float3 currPos = tex2D(tex1, TextureUV);

	float3 vec0 = g_attract0.xyz - currPos;
	float3 vec1 = g_attract1.xyz - currPos;
	float3 vec2 = g_attract2.xyz - currPos;

	vec0 = normalize(vec0) / length(vec0);
	vec1 = normalize(vec1) / length(vec1);
	vec2 = normalize(vec2) / length(vec2);

	float3 mergeForce = vec0 + vec1 + vec2;
	//mergeForce = normalize(mergeForce);

	float3 nextPos = prevVel + mergeForce * 2.0 * g_timeVar.x;

	Output.RGBColor.xyz = nextPos;

	return Output;
}

PS_OUTPUT Eular_UpdatePS( float2 TextureUV : TEXCOORD0 ) 
{ 
	PS_OUTPUT Output = (PS_OUTPUT)0;

	// Lookup mesh texture and modulate it with diffuse
	float3 prevVel = tex2D(tex0, TextureUV);
	float4 prevPos = tex2D(tex1, TextureUV);

	float3 nextPos = prevPos.xyz + prevVel * g_timeVar.x;

	Output.RGBColor.xyz = nextPos;
	Output.RGBColor.w = prevPos.w;
	return Output;
}

technique Verlet_StaticForce
{
	pass P0
	{
		VertexShader = null;
		PixelShader  = compile ps_3_0 Verlet_StaticForcePS(); 
	}
};

technique Eular_Attraction
{
	pass P0
	{
		VertexShader = null;
		PixelShader  = compile ps_3_0 Eular_AttractionPS(); 
	}
};

technique Eular_Update
{
	pass P0
	{
		VertexShader = null;
		PixelShader  = compile ps_3_0 Eular_UpdatePS(); 
	}
};

PS_OUTPUT Trail_MergePS( float2 TextureUV : TEXCOORD0 ) 
{
	PS_OUTPUT Output = (PS_OUTPUT)0.9;

	float3 prevVel = tex2D(tex0, TextureUV);

	Output.RGBColor.xyz = prevVel;

	return Output;
}

technique Trail_Merge
{
	pass P0
	{
		VertexShader = null;
		PixelShader  = compile ps_3_0 Trail_MergePS(); 
	}
};
