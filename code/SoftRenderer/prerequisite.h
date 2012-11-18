/**
  @file prerequisite.h
  
  @brief ǰ�ᣬ����ͷ�ļ���������ļ�

  @author yikaiming

  ������־ history
  ver:1.0
   
 */

#ifndef prerequisite_h__
#define prerequisite_h__

//////////////////////////////////////////////////////////////////////////
// ȫ�ֿ���

// ��դ��ͬ��_�������̹߳�դ��������ɵ�Z��ͻ
#define RASTERIZER_SYNC
// �̶�������դ������
//#define FIXED_FUNCTION_RASTERIZOR
// SIMD����
#define SR_USE_SIMD

//////////////////////////////////////////////////////////////////////////
// stl
#include <iostream>
// �޸İ汾��vector��֧���ڴ����
#include "myvector"
#include <map>
#include <string>
#include <stack>
#include <list>

//////////////////////////////////////////////////////////////////////////
// assert
#if defined( SR_PROFILE ) && defined ( NDEBUG )	// ����RELEASEģʽ�£�����SR_PROFILEʱ����assert
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

// �ڴ����
#define SR_ALIGN _CRT_ALIGN(16)

// fastcall����
#define SRFASTCALL __fastcall

//////////////////////////////////////////////////////////////////////////
// ���߰�
#include "util/math_def.h"
#include "util/pathutil.h"
#include "util/timer.h"
#include "util/thread.h"
#include "util/memfile.h"
#include "util/event.h"

//////////////////////////////////////////////////////////////////////////
// ��Ⱦ�������
#define SR_MAX_TEXTURE_STAGE_NUM 128
#define FBUFFER_CHANNEL_SIZE 4
#define SR_GREYSCALE_CLEARCOLOR 0x0
#define SR_SHADER_CONSTANTS_NUM 64

// �����դ���ֿ����
// ��block��С������ÿ���߳�TASK�ַ���task����
// ��Ҫ��β��Եõ�һ���Ϻõ�ֵ
#define VERTEX_TASK_BLOCK 64			///< VertexShader����ֿ��С
#define PIXEL_TASK_BLOCK 64				///< PixelShader����ֿ��С
#define RASTERIZE_TASK_BLOCK 512		///< Rasterize����ֿ��С

//////////////////////////////////////////////////////////////////////////
// UI��ʾɫ��
#define SR_UICOLOR_HIGHLIGHT 0xffffff
#define SR_UICOLOR_MAIN		 0xaf7333
#define SR_UICOLOR_NORMAL    0xcccccc


//////////////////////////////////////////////////////////////////////////
// ��ǰ����
class IRenderer;
class SrInputManager;
struct SrRendPrimitve;
class SrProfiler;
struct SrShaderContext;
struct SrMaterial;
class IResourceManager;
class SrTexture;
class SrScene;
class SrSwShader;
class SrResource;
class SrMesh;
class SrShader;
class SrDefaultMediaPack;

//////////////////////////////////////////////////////////////////////////
// ȫ�ֻ���

/**
 *@brief ȫ�ֻ���������ȫ�ֻ���ȡ����Ҫģ��ָ��
 */
struct GlobalEnvironment
{
	IRenderer*				renderer;
	SrTimer*				timer;
	SrInputManager*			inputSys;
	SrProfiler*				profiler;
	IResourceManager*		resourceMgr;
	SrScene*				sceneMgr;
};
extern GlobalEnvironment* gEnv;


//////////////////////////////////////////////////////////////////////////
// ö��

/**
 *@brief VB��ʽ, ��ʱֻʹ��P3N3T2
 */
enum EVBFormat
{
	// data SR_ALIGN struct
	eVBf_P3N3T2 = 0,
	eVBf_P3N3G3T2,

	// rend SR_ALIGN struct
	eVBf_P4N4T4 = 10,
	eVBf_P4N4G4T4 = 11,	

	eVBf_Max,
};

/**
 *@brief ������
 */
enum EMatrixDefine
{
	eMd_WorldViewProj = 0,
	eMd_World,
	eMd_View,
	eMd_Projection,
	eMd_WorldInverse,
	eMd_ViewInverse,
	eMd_Count,
};

/**
 *@brief ��դ����ʽ
 */
enum ERasterizeMode
{
	eRm_Solid,
	eRm_WireFrame,
	eRm_Point,
};

/**
 *@brief �����˾�
 */
enum ESamplerFilter
{
	eSF_Nearest,		///< �ٽ������
	eSF_Linear,			///< ˫���Թ���
};

/**
 *@brief ��Ⱦ��״̬
 */
enum ERenderingState
{
	eRs_Rendering = 1<<0,
	eRS_Locked = 1<<1,
	eRS_Swaping = 1<<2,
};

/**
 *@brief ��Ⱦ����
 */
enum ERenderFeature
{
	eRFeature_JitAA = 1<<0,						///< ���������
	eRFeature_MThreadRendering = 1<<1,			///< ���߳���Ⱦ
	eRFeature_LinearFiltering = 1<<2,			///< ˫���Բ���
	eRFeature_DotCoverageRendering = 1<<3,		///< Dot�ն���Ⱦ
	eRFeature_InterlaceRendering = 1<<4,		///< Dot�ն���Ⱦ
};

/**
 *@brief ��Դ����
 */
enum EResourceType
{
	eRt_Mesh = 0,
	eRT_Texture,
	eRT_Material,
	eRT_SwShader,
	eRT_HwShader,

	eRT_Count,
};

enum EShaderConstantsSlot
{
	eSC_VS0 = 0,
	eSC_VS1,
	eSC_VS2,
	eSC_VS3,

	eSC_PS0 = SR_SHADER_CONSTANTS_NUM,
	eSC_PS1,
	eSC_PS2,
	eSC_PS3,

	eSC_ShaderConstantCount = SR_SHADER_CONSTANTS_NUM * 2,
};
//////////////////////////////////////////////////////////////////////////
// ������Ⱦ�ṹ����

/**
 *@brief ͨ��objģ�͵Ķ����ʽ
 */
SR_ALIGN struct SrVertexP3N3T2
{
	float4 pos;

	// vertex shader usage
	float3 normal;
	float2 texcoord;
};

/**
 *@brief ��Ⱦ����, DrawPrimitveʱ����Primitive�����¶���
 */
SR_ALIGN struct SrRendVertex
{
	float4 pos;

	// vertex shader usage
	float4 channel1;
	float4 channel2;
	float4 channel3;
	// 	float4 channel6;
	// 	float4 channel7;
};

/**
 *@brief ����buffer�Ľṹ����
 */
SR_ALIGN struct SrFragment
{
	// preserve a 4-channel data SR_ALIGN struct for GBuffer
	union{
		SR_ALIGN float data[FBUFFER_CHANNEL_SIZE * 4];
		struct  
		{
			float4 hpos;
			float4 worldpos_tx;
			float4 normal_ty;
			float4 preserve;
		};
	};	
	///< 4 x float4 �����ݷ�Χ
	SrRendPrimitve*	primitive;	///< primitive����
};

struct SrFragmentBufferSync
{
	SrFragmentBufferSync() {
		InitializeCriticalSection(&cs);
		//InitializeSRWLock(&srwLock);
	}
	~SrFragmentBufferSync() {
		DeleteCriticalSection(&cs);
	}
	CRITICAL_SECTION cs;
	//SRWLOCK srwLock;
};

/**
 *@brief �����޹ص�VertexBuffer�ṹ 
 */
SR_ALIGN struct SrVertexBuffer
{
	uint8* data;
	uint32 elementSize;
	uint32 elementCount;

	void* userData;

	SrVertexBuffer()
	{
		memset(this, 0, sizeof(SrVertexBuffer));
	}

	void setData( void* srcData, uint32 index )
	{
		if (index < elementCount)
		{
			memcpy( data + elementSize * index, srcData, elementSize);
		}
	}
};

/**
 *@brief �ӿڽṹ
 */
SR_ALIGN struct SrViewport
{
	float x,y,w,h,n,f;

	SrViewport() {}
	SrViewport(float px, float py, float pw, float ph, float pn, float pf) :
		x(px),
		y(py),
		w(pw),
		h(ph),
		n(pn),
		f(pf)
	{

	}
};

/**
 *@brief IndexBuffer�ṹ
 */
SR_ALIGN struct SrIndexBuffer
{
	uint32* data;
	uint32	count;
	uint32  currAddress;

	void* userData;

	SrIndexBuffer()
	{
		data = 0;
		count = 0;
		currAddress = 0;
		userData = 0;
	}
	void setData( uint32 srcData, uint32 index )
	{
		if (index < count)
		{
			data[index] = srcData;
		}
	}

	void Clear()
	{
		memset(data, 0, sizeof(uint32) * count);
		currAddress = 0;
	}

	void push_back(uint32 index)
	{
		data[currAddress++] = index;
		assert( currAddress <= count );
	}

	uint32 size()
	{
		return currAddress;
	}
};

/**
 *@brief ������ṹ
 */
SR_ALIGN struct SrPrimitve
{
	SrVertexBuffer*		vb;			///< vertex buffer
	SrIndexBuffer*		ib;			///< index buffer

	SrVertexBuffer*		cachedVb;

	SrMaterial*			material;	///< ����
};

/**
 *@brief �ƹ�ṹ
 */
SR_ALIGN struct SrLight
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;

	float3 worldPos;
	float radius;
	float attenuation;

	void * operator new(size_t size) {return _mm_malloc(size, 16);}
	void operator delete(void *memoryToBeDeallocated) {_mm_free(memoryToBeDeallocated);}
};


//////////////////////////////////////////////////////////////////////////
// �ṹ���ж���
typedef std::vector<SrPrimitve> SrPrimitives;					///< ���������
typedef std::vector<SrLight*> SrLightList;						///< �ƹ����
typedef std::vector<float44> SrMatrixArray;						///< �������
typedef std::vector<const SrTexture*> SrBitmapArray;			///< ������ʶ���

/**
 *@brief ͨ��Constant Buffer
 *@remark cBuffer���ֻ֧��8��float4, ��32���������������ʱ��ע���С��
 */
struct SrPixelShader_Constants
{
	float4 difColor;
	float4 spcColor;
	float glossness; float fresnelPower; float fresnelBia; float fresnelScale;
};

struct IResourceManager
{
	virtual SrMesh*				LoadMesh(const char* filename) =0;
	virtual const SrTexture*	LoadTexture(const char* filename, bool bump = false) =0;
	virtual SrMaterial*			LoadMaterial(const char* filename) =0;
	virtual SrMaterial*			CreateMaterial(const char* filename) =0;
	virtual void				LoadMaterialLib(const char* filename) =0;
	virtual SrShader*			GetShader(const char* name) =0;
	virtual void				AddShader(SrShader* shader) =0;
	virtual void				ReloadShaders() =0;

	virtual SrTexture*			CreateRenderTexture(const char* name, int width, int height, int bpp) =0;
	virtual SrMaterial*			CreateManmualMaterial(const char* name) =0;

	virtual void				InitDefaultMedia() =0;
	virtual SrDefaultMediaPack*	getDefaultMediaPack() =0;
};

#endif // prerequisite_h__