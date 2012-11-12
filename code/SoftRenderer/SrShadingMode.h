/**
  @file SrShadingMode.h
  
  @author yikaiming

  @breif 着色模式抽象基类以及基础flat, gourand, phong着色模式的实现类

  更改日志 history
  ver:1.0
   
 */

#ifndef SrShadingMode_h__
#define SrShadingMode_h__

#include "prerequisite.h"
#include "SrResource.h"

/**
 *@brief ShadingMode基类
 *@remark Shader类，涵盖物体的着色策略。其中VS,RS,PS每种着色必须实现
   PATCH SHADER为光栅化前对独立三角面片的处理机会，需要处理这一阶段的着色策略可以选择实现。
 */
class SrShader : public SrResource
{
public:
	SrShader(const char* name):SrResource(name, eRT_Shader) {}
	virtual ~SrShader(void) {}

	/**
	 *@brief Patch Shader, 在光栅化前，对三角面三个顶点进行重新运算的着色器。继承的着色策略按需实现。
	 *@return void 
	 *@param void * vOut 
	 *@param void * vOut1 
	 *@param void * vOut2 
	 *@param const void * vInRef0 
	 *@param const void * vInRef1 
	 *@param const void * vInRef2 
	 *@param const SrShaderContext * context 
	 */
	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context ) {}
	virtual void SRFASTCALL ProcessVertex(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context) =0;
	virtual void SRFASTCALL ProcessRasterize(void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false) = 0;
	virtual void SRFASTCALL ProcessPixel(uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address) =0;
};

class SrFlatShader : public SrShader
{
public:
	SrFlatShader():SrShader("FlatShader") {}
	~SrFlatShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};

class SrGourandShader : public SrShader
{
public:
	SrGourandShader():SrShader("GourandShader") {}
	~SrGourandShader() {}

	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};

class SrPhongShader : public SrShader
{
public:
	SrPhongShader():SrShader("PhongShader") {}
	~SrPhongShader() {}

	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context , uint32 address);
};


class SrPhongWithNormalShader : public SrShader
{
public:
	SrPhongWithNormalShader():SrShader("PhongWithNormalMapShader") {}
	~SrPhongWithNormalShader() {}

	virtual void SRFASTCALL ProcessPatch(void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessVertex( void* vOut, void* vOut1, void* vOut2, const void* vInRef0, const void* vInRef1, const void* vInRef2, const SrShaderContext* context );
	virtual void SRFASTCALL ProcessRasterize( void* rOut, const void* rInRef0, const void* rInRef1, const void* rInRef2, float ratio, const SrShaderContext* context, bool final = false );
	virtual void SRFASTCALL ProcessPixel( uint32* pOut, const void* pIn, const SrShaderContext* context, uint32 address );
};


// GLOBAL SHADER
extern SrPhongShader g_PhongShadingShader;
extern SrFlatShader g_FlatShadingShader;
extern SrGourandShader g_GourandShadingShader;
extern SrPhongWithNormalShader g_PhongShadingWithNormalShader;
void LoadInternalShaders();
#endif // SrShadingMode_h__


