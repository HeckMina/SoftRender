/**
  @file SrCustomShader.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrCustomShader_h__
#define SrCustomShader_h__

#include "prerequisite.h"
#include "SrShadingMode.h"

class SrSkinSimShader : public SrShader
{
public:
	SrSkinSimShader():SrShader("SkinSimShader") {}
	~SrSkinSimShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};

extern SrSkinSimShader g_SkinSimShader;

class SrFresnelNormalShader : public SrShader
{
public:
	SrFresnelNormalShader():SrShader("FresnelWithNormalShader") {}
	~SrFresnelNormalShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};

extern SrFresnelNormalShader g_FresnelNormalShader;

class SrHairShader : public SrShader
{
public:
	SrHairShader():SrShader("HairShader") {}
	~SrHairShader() {}

	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};

extern SrHairShader g_HairShader;

void LoadCustomShaders();

#endif // SrCustomShader_h__