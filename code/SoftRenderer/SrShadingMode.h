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
#include "SrResource.h"

/**
 *@brief ShadingMode����
 *@remark Shader�࣬�����������ɫ���ԡ�����VS,RS,PSÿ����ɫ����ʵ��
   PATCH SHADERΪ��դ��ǰ�Զ���������Ƭ�Ĵ�����ᣬ��Ҫ������һ�׶ε���ɫ���Կ���ѡ��ʵ�֡�
 */
class SrShader : public SrResource
{
public:
	SrShader(const char* name):SrResource(name, eRT_Shader) {}
	virtual ~SrShader(void) {}

	/**
	 *@brief Patch Shader, �ڹ�դ��ǰ��������������������������������ɫ�����̳е���ɫ���԰���ʵ�֡�
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


