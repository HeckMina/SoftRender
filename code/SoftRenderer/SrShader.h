/**
  @file SrShader.h
  
  @author Kaiming

  ������־ history
  ver:1.0
   
 */

#ifndef SrShader_h__
#define SrShader_h__

#include "prerequisite.h"
#include "SrResource.h"
#include "SrTexture.h"
/**
 *@brief Shader�����Ľṹ�����ڻ���Flush�õ�ÿһ��primitive����Ⱦ����
 */
SR_ALIGN struct SrShaderContext
{
	float4		  shaderConstants[eSC_ShaderConstantCount];
	SrMatrixArray matrixs;
	SrBitmapArray textureStage;
	SrLightList	  lightList;	
	bool		  alphaTest;
	bool		  culling;

	const void* GetPixelShaderConstantPtr() const
	{
		return &(shaderConstants[eSC_PS0]);
	}

	uint32 Tex2D(float2& texcoord, uint32 stage) const
	{
		uint32 ret = 0x00000000;
		if (stage < textureStage.size() && textureStage[stage] != 0)
		{
			ret = gEnv->renderer->Tex2D(texcoord, textureStage[stage]);
		}
		return ret;
	}
};

class SrShader : public SrResource
{
public:
	SrShader(const char* name):SrResource(name, eRT_SwShader) {}
	virtual ~SrShader(void) {}
};

#endif