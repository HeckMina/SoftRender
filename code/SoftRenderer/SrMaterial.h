/**
  @file SrMaterial.h
  
  @author Kaiming

  ������־ history
  ver:1.0
   
 */

#ifndef SrMaterial_h__
#define SrMaterial_h__
#include "prerequisite.h"
#include "SrResource.h"

class SrShader;

struct SrMaterial : public SrResource
{
	SrMaterial(const char* name);
	~SrMaterial() {}

	void LoadFromFile();
	void SetShader( SrShader* shader ) {m_shader = shader;}
	void ApplyTextures() const;
	void ApplyShaderConstants() const;

	float4			m_matDiffuse;
	float4			m_matSpecular;

	union
	{
		struct  
		{
			float			m_glossness;
			float			m_fresnelPower;
			float			m_fresnelBia;
			float			m_fresnelScale;
		};
		float				channel2[4];
	};

	SrShader*		m_shader;
	SrBitmapArray	m_textures;			///< ��������
	


	bool	m_alphaTest;

	void * operator new(size_t size) {return _mm_malloc(size, 16);}
	void operator delete(void *memoryToBeDeallocated) {_mm_free(memoryToBeDeallocated);}
};

#endif