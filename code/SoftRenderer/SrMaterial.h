/**
  @file SrMaterial.h
  
  @author Kaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrMaterial_h__
#define SrMaterial_h__
#include "prerequisite.h"
#include "SrResource.h"

class SrShader;

SR_ALIGN struct SrMaterial : public SrResource
{
	SrMaterial(const char* name);
	~SrMaterial() {}

	void LoadFromFile();
	void SetShader( SrShader* shader ) {m_shader = shader;}
	void ApplyTextures() const;

	SrShader*		m_shader;
	SrBitmapArray		m_textures;			///< 材质纹理

	float4				m_cbuffer[8];		///< cbuffer

	bool	m_alphaTest;

	void * operator new(size_t size) {return _mm_malloc(size, 16);}
	void operator delete(void *memoryToBeDeallocated) {_mm_free(memoryToBeDeallocated);}
};

#endif