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

struct IDirect3DTexture9;

typedef std::vector<gkGpuTimer> SrGpuTimers;
typedef std::vector<IDirect3DTexture9*> SrHwTextures;

class SrHwD3D9Renderer :public IRenderer
{
public:
	SrHwD3D9Renderer(void);
	~SrHwD3D9Renderer(void);

	virtual bool InnerInitRenderer( HWND hWnd, int width, int height, int bpp );

	virtual bool InnerShutdownRenderer();

	virtual bool Resize( uint32 width, uint32 height );

	virtual void BeginFrame();

	virtual void EndFrame();

	virtual bool HwClear();

	virtual void* getBuffer();

	virtual bool SetTextureStage( const SrTexture* texture, int stage );
	virtual void ClearTextureStage();

	
	virtual bool DrawPrimitive( SrPrimitve* primitive );
	virtual bool DrawLine( const float3& from, const float3& to );

	virtual bool SetShader( const SrShader* shader );
	virtual bool SetShaderConstant( EShaderConstantsSlot slot, const float* constantStart, uint32 vec4Count );
	
private:
	void FlushText();

	virtual SrVertexBuffer* AllocateVertexBuffer( uint32 elementSize, uint32 count, bool fastmode = false );

	virtual bool DeleteVertexBuffer( SrVertexBuffer* target );

	virtual SrIndexBuffer* AllocateIndexBuffer( uint32 count );

	virtual bool DeleteIndexBuffer( SrIndexBuffer* target );

	virtual bool UpdateVertexBuffer( SrVertexBuffer* target );

	virtual bool UpdateIndexBuffer( SrIndexBuffer* target );

	virtual void SetGpuMarkStart( EHwTimerElement element );

	virtual void SetGpuMarkEnd( EHwTimerElement element );

	virtual float GetGpuTime( EHwTimerElement element );

	virtual uint32 Tex2D( float2& texcoord, const SrTexture* texture ) const;



	class SrHwTextFlusher* m_textFlusher;

	struct IDirect3D9* m_d3d9;
	struct IDirect3DDevice9* m_hwDevice;
	struct IDirect3DVertexShader9* m_defaultVS;
	struct IDirect3DVertexShader9* m_rhzVS;

	struct IDirect3DPixelShader9* m_defaultPS;
	struct IDirect3DPixelShader9* m_jitaaPS;

	struct IDirect3DVertexDeclaration9* m_defaultVertexDecl;
	struct IDirect3DVertexDeclaration9* m_rhzVertexDecl;

	struct IDirect3DTexture9* m_backBuffer0;
	struct IDirect3DTexture9* m_backBuffer1;
	struct IDirect3DSurface9* m_depthStencil;
	struct IDirect3DSurface9* m_backBuffer;

	SrHwTextures m_hwTextures;

	SrGpuTimers m_gpuTimers;
};

#endif // SrHwD3D9Renderer_h__


