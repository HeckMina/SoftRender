/**
  @file IRenderer.h
  
  @brief ��������Ⱦ�ṹ����

  @author yikaiming

  ������־ history
  ver:1.0
   
 */

#ifndef IRenderer_h__
#define IRenderer_h__

#include "prerequisite.h"

class SrTexture;
class SrRasterizer;
class SrResourceManager;

struct SrTextLine
{
	std::string text;
	int2 pos;
	uint32 color;
	uint32 size;
};
typedef std::vector<SrTextLine> SrTextLines;
typedef std::vector<SrVertexBuffer*> SrVertexBufferArray;		///< VB����
typedef std::vector<SrIndexBuffer*> SrIndexBufferArray;			///< IB����

enum EHwTimerElement
{
	eHt_GpuFlushTime = 0,
	eHt_GpuHairTime,
	eHt_GpuPostProcessTime,

	eHt_Count,
};

enum ERendererType
{
	eRt_Software,
	eRt_HardwareD3D9,
	eRt_HardwareD3D11,
};

// Renderer Interface
class IRenderer
{
public:
	IRenderer(ERendererType type);
	virtual ~IRenderer(void);

	// �������رպ���
	bool InitRenderer(HWND hWnd, int width, int height, int bpp);
	bool ShutdownRenderer();
	virtual bool Resize(uint32 width, uint32 height)=0;

	// ֡���ƺ���
	virtual void BeginFrame()=0;
	virtual void EndFrame()=0;

	// Ӳ��Clear
	virtual bool HwClear()=0;

	// ��ȡScreenBuffer����
	virtual void* getBuffer() =0;

	// ����ͨ������
	virtual bool SetTextureStage( const SrTexture* texture, int stage )=0;
	virtual void ClearTextureStage()=0;

	// Buffer����
	virtual SrVertexBuffer* AllocateVertexBuffer(uint32 elementSize, uint32 count, bool fastmode = false);
	virtual bool DeleteVertexBuffer(SrVertexBuffer* target);
	virtual SrIndexBuffer*	AllocateIndexBuffer(uint32 count);
	virtual bool DeleteIndexBuffer(SrIndexBuffer* target);

	virtual SrVertexBuffer* AllocateNormalizedVertexBuffer( uint32 count, bool fastmode = false ) {return NULL;}

	virtual bool UpdateVertexBuffer(SrVertexBuffer* target) {return true;}
	virtual bool UpdateIndexBuffer(SrIndexBuffer* target) {return true;}

	bool DrawScreenText(const char* str, int x,int y, uint32 size, DWORD color = SR_UICOLOR_HIGHLIGHT);

	// ��Ⱦ����
	virtual bool DrawPrimitive( SrPrimitve* primitive )=0;
	virtual bool DrawLine(const float3& from, const float3& to)=0;

	virtual void SetGpuMarkStart(EHwTimerElement element) {}
	virtual void SetGpuMarkEnd(EHwTimerElement element) {}
	virtual float GetGpuTime(EHwTimerElement element) {return 0;}

	// ������
	void SetMatrix( EMatrixDefine index, const float44& matrix);
	float44 GetMatrix( EMatrixDefine index );

	// FrameCount
	uint32 getFrameCount() {return m_frameCount;}

	ERendererType m_rendererType;

protected:
	virtual bool InnerInitRenderer(HWND hWnd, int width, int height, int bpp) =0;
	virtual bool InnerShutdownRenderer() =0;


protected:

	SrVertexBufferArray m_vertexBuffers;
	SrIndexBufferArray	m_indexBuffers;
	SrMatrixArray m_matrixStack;



	uint32 m_frameCount;

	SrTextLines m_textLines;

	HWND m_hWnd;

	
};



#endif // IRenderer_h__