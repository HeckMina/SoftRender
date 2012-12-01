/**
  @file SrHwD3D9Renderer.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrHwD3D9Renderer_h__
#define SrHwD3D9Renderer_h__
#include "prerequisite.h"
#include "SrGpuTimer.h"
#include "RendererBase.h"

typedef std::vector<gkGpuTimer> SrGpuTimers;

struct IDirect3DTexture9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
class SrHwRenderTexture;
class SrHwShader;


class SrHwD3D9Renderer : public IRenderer
{
	enum EInternalShader
	{
		eInS_line,
		eInS_dof,
		eInS_jitaa,
		eInS_blur,

		eInS_Count
	};

	enum ERenderTarget
	{
		eRtt_Backbuffer0 = 0,
		eRtt_Backbuffer1,
		eRtt_BackbufferHalf0,
		eRtt_BackbufferHalf1,
		eRtt_BackbufferBlur,

		eRtt_Depth,

		eRtt_Count
	};

	enum EDSSize
	{
		eDSS_Full,
		eDSS_Half,
		eDSS_Quad,
	};


	typedef std::vector<IDirect3DTexture9*> SrHwTextures;
	typedef std::vector<IDirect3DVertexBuffer9*> SrBindVBs;
	typedef std::vector<IDirect3DIndexBuffer9*> SrBindIBs;
	typedef std::vector<SrHwRenderTexture*> SrHwRenderTextures;
	typedef std::stack<SrHwRenderTexture*> SrHwRTStack;
	typedef std::vector<SrHwShader*> SrHwShaders;

	
public:
	SrHwD3D9Renderer(void);
	~SrHwD3D9Renderer(void);

	virtual const char* getName();

	virtual bool InitRenderer( HWND hWnd, int width, int height, int bpp );

	virtual bool ShutdownRenderer();

	virtual bool Resize( uint32 width, uint32 height );

	virtual void BeginFrame();

	virtual void EndFrame();

	virtual bool HwClear();

	virtual void* getBuffer();

	virtual bool SetTextureStage( const SrTexture* texture, int stage );
	virtual void ClearTextureStage();

	
	virtual bool DrawPrimitive( SrPrimitve* primitive );
	virtual bool DrawLine( const float3& from, const float3& to );
	bool DrawScreenText(const char* str, int x,int y, uint32 size, DWORD color = SR_UICOLOR_HIGHLIGHT);
	
	virtual bool SetHwShader( SrHwShader* shader );

	virtual bool SetShader( const SrShader* shader );

	virtual bool SetShaderConstant( uint32 slot, const float* constantStart, uint32 vec4Count );

	virtual uint32 Tex2D( float2& texcoord, const SrTexture* texture ) const;

	virtual bool UpdateShaderConstantsPerFrame();


private:
	void FlushText();

	virtual bool UpdateVertexBuffer( SrVertexBuffer* target );

	virtual bool UpdateIndexBuffer( SrIndexBuffer* target );

	virtual void SetGpuMarkStart( EHwTimerElement element );

	virtual void SetGpuMarkEnd( EHwTimerElement element );

	virtual float GetGpuTime( EHwTimerElement element );

	virtual void SetVertexShaderConstants( uint32 startIdx, const float* data, uint32 vec4Count );

	void RP_ProcessDOF();
	void FX_GaussisanBlur( SrHwRenderTexture* texture, float disort, float scale, int iterate );


	void DrawScreenQuad( SrTexture* texture = NULL );
	void PushRenderTarget( uint8 index, SrHwRenderTexture* texture );
	void PopRenderTarget( uint8 index );
	void SetRenderTarget( uint8 index, SrHwRenderTexture* texture );

	void StretchRT2TEX( uint8 index, SrHwRenderTexture* texture );
	void StretchTEX2TEX( SrHwRenderTexture* source, SrHwRenderTexture* target );

	void SetSamplerState( uint8 stage, DWORD state, DWORD value);
	void SetRenderState( DWORD state, DWORD value );

	


	//class SrHwTextFlusher* m_textFlusher;

	struct IDirect3D9* m_d3d9;
	struct IDirect3DDevice9* m_hwDevice;

	struct IDirect3DVertexDeclaration9* m_defaultVertexDecl;
	struct IDirect3DVertexDeclaration9* m_skinVertexDecl;

	struct IDirect3DVertexDeclaration9* m_rhzVertexDecl;
	struct IDirect3DVertexDeclaration9* m_lineVertexDecl;

		
	struct IDirect3DSurface9* m_backBuffer;

	SrGpuTimers m_gpuTimers;

	SrHwTextures m_hwTextures;
	SrHwRenderTextures m_hwRTs;

	std::vector<float3> m_drawlines;

	SrHwRTStack m_RTStack[4];
	SrHwShaders m_hwShaders;

	struct IDirect3DSurface9* m_depthStencil;
	struct IDirect3DSurface9* m_depthStencilHalf;
	EDSSize m_DSSize;

	SrBindVBs m_bindVBs;
	SrBindIBs m_bindIBs;

	HFONT m_bigFont;
	HFONT m_smallFont;
	SrTextLines m_textLines;
};

#endif // SrHwD3D9Renderer_h__


