/**
  @file SrRenderer.cpp
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#include "StdAfx.h"

#include <d3d9.h>

#include "SrSoftRenderer.h"
#include "srBitmap.h"
#include "SrRasterizer.h"
#include "SrProfiler.h"
#include "SrShadingMode.h"

#include "mmgr/mmgr.h"

#define SR_NORMALIZE_VB_MAX_SIZE 1024 * 1024 * 10

SrRendContext* g_context = NULL;

SrSoftRenderer::SrSoftRenderer(void):IRenderer(eRt_Software),
	m_d3d9(NULL),
	m_drawSurface(NULL),
	m_hwDevice(NULL),
	m_cachedBuffer(NULL),
	m_bufferPitch(0),
	m_renderState(0)
{
	m_textureStages.assign( SR_MAX_TEXTURE_STAGE_NUM , NULL );

	// create HFONT
	LOGFONT lfont;
	memset   (&lfont,   0,   sizeof   (LOGFONT));   
	lfont.lfHeight=14;
	lfont.lfWeight=800;   
	lfont.lfClipPrecision=CLIP_LH_ANGLES; 
	lfont.lfQuality = NONANTIALIASED_QUALITY; // THIS COULD BOOST
	strcpy_s( lfont.lfFaceName, "consolas" );
	m_bigFont = CreateFontIndirect( &lfont );

	lfont.lfHeight=12;
	lfont.lfWeight=0;  
	m_smallFont = CreateFontIndirect( &lfont );
	m_normalizeVBAllocSize = 0;
}


SrSoftRenderer::~SrSoftRenderer(void)
{
	DeleteObject(m_smallFont);
	DeleteObject(m_bigFont);
}

bool SrSoftRenderer::InnerInitRenderer( HWND hWnd, int width, int height, int bpp )
{
	//////////////////////////////////////////////////////////////////////////
	// create d3d device for Show Soft Buffer
	if( NULL == ( m_d3d9 = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return false;

	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// bpp 32 XRGB
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;

	// Create the D3DDevice
	if( FAILED( m_d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &m_hwDevice ) ) )
	{
		MessageBox(hWnd,"无法创建DX设备","",MB_OK);
		return false;
	}

	// 关闭Z BUFFER
	m_hwDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_hwDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	// 创建硬件buffer
	CreateHwBuffer();

	// 创建光栅化处理器
	m_rasterizer = new SrRasterizer;
	m_rasterizer->Init(this);

	m_normalizeVertexBuffer = (SrRendVertex*)_mm_malloc( SR_NORMALIZE_VB_MAX_SIZE * sizeof(SrRendVertex), 16 );

	return true;
}

bool SrSoftRenderer::InnerShutdownRenderer()
{
	_mm_free(m_normalizeVertexBuffer);

	delete m_rasterizer;

	if (m_drawSurface != NULL)
	{
		m_drawSurface->Release();
	}

	if( m_hwDevice != NULL )
		m_hwDevice->Release();

	if( m_d3d9 != NULL )
		m_d3d9->Release();

	return true;
}

bool SrSoftRenderer::HwClear()
{
// 	if (!m_drawSurface)
// 		return false;
// 
// 	HRESULT res = m_hwDevice->ColorFill(m_drawSurface,0,D3DCOLOR_ARGB(255,50,50,50));
// 	if (res!=S_OK)
// 	{
// 		return false;
// 	}

	return true;
}

bool SrSoftRenderer::Swap()
{
	m_renderState |= eRS_Swaping;
	IDirect3DSurface9* bbSurf;

	m_hwDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bbSurf);
	m_hwDevice->StretchRect( m_drawSurface, NULL, bbSurf, NULL, D3DTEXF_NONE );
	m_hwDevice->Present(NULL, NULL, NULL, NULL);

	bbSurf->Release();
	m_renderState &= ~eRS_Swaping;
	return true;
}

bool SrSoftRenderer::CreateHwBuffer()
{
	if (m_drawSurface == NULL)
	{
		IDirect3DSurface9* bbSurf;
		m_hwDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bbSurf);

		D3DSURFACE_DESC bbDesc;
		bbSurf->GetDesc( &bbDesc );

		m_hwDevice->CreateOffscreenPlainSurface( g_context->width, g_context->height, 
			bbDesc.Format,
			D3DPOOL_DEFAULT,
			&m_drawSurface,NULL);

		bbSurf->Release();
	}
	return true;
}

void SrSoftRenderer::BeginFrame()
{
	m_renderState |= eRs_Rendering;
	m_frameCount++;

	ClearTextureStage();
}

void SrSoftRenderer::EndFrame()
{
// 	if ( !(m_renderState & eRS_Locked))
// 	{
		D3DLOCKED_RECT lockinfo;
		memset(&lockinfo,0,sizeof(lockinfo));

		HRESULT res = m_drawSurface->LockRect(&lockinfo,NULL,D3DLOCK_DISCARD);
		if (res!=S_OK)
		{
			// FATAL ERROR
			return;			
		}

		m_cachedBuffer = lockinfo.pBits;
		m_bufferPitch = lockinfo.Pitch;
		m_rasterizer->Flush();

		m_drawSurface->UnlockRect();

		FlushText();

		m_textLines.clear();

	m_renderState &= ~eRs_Rendering;

	Swap();


	for (uint32 i=0; i < m_normlizedVBs.size(); ++i)
	{
		delete m_normlizedVBs[i];
	}
	m_normlizedVBs.clear();


	m_normalizeVBAllocSize = 0;
}

void* SrSoftRenderer::getBuffer()
{
	return m_cachedBuffer;
}

bool SrSoftRenderer::SetTextureStage( const SrTexture* texture, int stage )
{
	if( texture && stage < SR_MAX_TEXTURE_STAGE_NUM )
	{
		m_textureStages[stage] = texture;
		return true;
	}
	
	return false;
}


void SrSoftRenderer::ClearTextureStage()
{
	for (uint32 i=0; i < m_textureStages.size(); ++i)
	{
		m_textureStages[i] = NULL;
	}
}

bool SrSoftRenderer::DrawPrimitive( SrPrimitve* primitive )
{
	return m_rasterizer->DrawPrimitive(primitive);
}

void SrSoftRenderer::FlushText()
{
	if (!m_drawSurface)
		return;

	HDC hdc ;
	HRESULT res = m_drawSurface->GetDC(&hdc);
	if (res!=S_OK)
	{
		return;
	}

	SrTextLines::iterator it = m_textLines.begin();
	for ( ; it != m_textLines.end(); ++it )
	{
		RECT rect;
		rect.left = it->pos.x;
		rect.right =(LONG)(strlen(it->text.c_str()) * 10 + it->pos.x);
		rect.top = it->pos.y;
		rect.bottom = it->pos.y+20;
		int len = (int)strlen(it->text.c_str());

		if ( it->size == 0)
		{
			SelectObject(hdc, m_bigFont);
		}
		else
		{
			SelectObject(hdc, m_smallFont);
		}
		
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, it->color);
		DrawTextA(hdc,it->text.c_str(),len,&rect,DT_LEFT);	
	}

	m_drawSurface->ReleaseDC(hdc);
}



bool SrSoftRenderer::Resize( uint32 width, uint32 height )
{
	return true;
}

bool SrSoftRenderer::DrawLine( const float3& from, const float3& to )
{
	m_rasterizer->DrawLine(from, to);
	return true;
}

SrVertexBuffer* SrSoftRenderer::AllocateNormalizedVertexBuffer( uint32 count, bool fastmode )
{
	if (fastmode)
	{
		SrVertexBuffer* ret = new SrVertexBuffer;
		ret->data = (uint8*)(m_normalizeVertexBuffer + m_normalizeVBAllocSize);
		ret->elementCount = count;
		ret->elementSize = sizeof(SrRendVertex);

		m_normlizedVBs.push_back(ret);
			
		m_normalizeVBAllocSize += count;

		return ret;
	}
	else
	{
		return AllocateVertexBuffer( sizeof(SrRendVertex), count);
	}	
}
