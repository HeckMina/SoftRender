/**
  @file SrRenderer.h
  
  @author yikaiming

  ������־ history
  ver:1.0
   
 */

#ifndef SrRenderer_h__
#define SrRenderer_h__
#include "prerequisite.h"
#include "RendererBase.h"

struct SrFragmentBuffer;

/**
 *@brief ��Ⱦ��
 */
class SrSoftRenderer : public IRenderer
{
	friend SrRasterizer;
public:
	SrSoftRenderer(void);
	virtual ~SrSoftRenderer(void);

	// �������رպ���
	bool InnerInitRenderer(HWND hWnd, int width, int height, int bpp);
	bool InnerShutdownRenderer();
	bool Resize(uint32 width, uint32 height);

	// ֡���ƺ���
	void BeginFrame();
	void EndFrame();

	// Ӳ��Clear
	bool HwClear();

	// ��ȡScreenBuffer����
	void* getBuffer();
	
	// ����ͨ������
	bool SetTextureStage( const SrTexture* texture, int stage );
	void ClearTextureStage();
	
	// ��Ⱦ����
	bool DrawPrimitive( SrPrimitve* primitive );
	bool DrawLine(const float3& from, const float3& to);

	// Shader����
	virtual bool SetShader( const SrShader* shader );
	virtual bool SetShaderConstant( EShaderConstantsSlot slot, const float* constantStart, uint32 vec4Count );

private:
	// ����Ӳ��֡����
	bool Swap();
	// ����Ӳ��buffer����
	bool CreateHwBuffer();
	// ������������
	void FlushText();

	virtual SrVertexBuffer* AllocateNormalizedVertexBuffer( uint32 count, bool fastmode = false );




	// DXӲ������
	struct IDirect3D9* m_d3d9;
	struct IDirect3DDevice9* m_hwDevice;
	struct IDirect3DSurface9* m_drawSurface;
	
	// ��դ������
	SrRasterizer* m_rasterizer;

	uint32 m_renderState;

	void* m_cachedBuffer;
	int m_bufferPitch;

	SrBitmapArray m_textureStages;

	HFONT m_bigFont;
	HFONT m_smallFont;

	SrRendVertex* m_normalizeVertexBuffer;
	uint32 m_normalizeVBAllocSize;
	SrVertexBufferArray m_normlizedVBs;

	const SrShader* m_currShader;
	float4* m_shaderConstants;
};

#endif // SrRenderer_h__


