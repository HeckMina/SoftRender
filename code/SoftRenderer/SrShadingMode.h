/**
  @file SrShadingMode.h
  
  @author yikaiming

  @breif ��ɫģʽ��������Լ�����flat, gourand, phong��ɫģʽ��ʵ����

  ������־ history
  ver:1.0
   
 */

#ifndef SrShadingMode_h__
#define SrShadingMode_h__

#include "prerequisite.h"
#include "shading.h"


class SrFlatShader : public SrSwShader
{
public:
	SrFlatShader():SrSwShader("FlatShader") {}
	~SrFlatShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false ) const;
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address ) const;
};

class SrGourandShader : public SrSwShader
{
public:
	SrGourandShader():SrSwShader("GourandShader") {}
	~SrGourandShader() {}

	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false ) const;
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address ) const;
};

class SrPhongShader : public SrSwShader
{
public:
	SrPhongShader():SrSwShader("PhongShader") {}
	~SrPhongShader() {}

	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false ) const;
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context , uint32 address) const;
};


class SrPhongWithNormalShader : public SrSwShader
{
public:
	SrPhongWithNormalShader():SrSwShader("PhongWithNormalMapShader") {}
	~SrPhongWithNormalShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) const;
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false ) const;
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address ) const;
};


// GLOBAL SHADER
extern SrPhongShader g_PhongShadingShader;
extern SrFlatShader g_FlatShadingShader;
extern SrGourandShader g_GourandShadingShader;
extern SrPhongWithNormalShader g_PhongShadingWithNormalShader;
void LoadInternalShaders();
#endif // SrShadingMode_h__

