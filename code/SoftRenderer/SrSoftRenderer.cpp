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
#include "SrSwShader.h"
#include <gl/GL.h>

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
	m_normalizeVBAllocSize = 0;
}


SrSoftRenderer::~SrSoftRenderer(void)
{
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

	m_shaderConstants = (float4*)_mm_malloc( eSC_ShaderConstantCount * sizeof(float4), 16 );

	InnerInitShaders();

	return true;
}

bool SrSoftRenderer::InnerShutdownRenderer()
{
	for (uint32 i=0; i < m_swHandles.size(); ++i)
	{
		FreeLibrary( m_swHandles[i] );
	}

	_mm_free(m_normalizeVertexBuffer);
	_mm_free(m_shaderConstants);

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

bool SrSoftRenderer::SetShader( const SrShader* shader )
{
	if (shader)
	{
		for ( uint32 i=0; i < m_swShaders.size(); ++i )
		{
			if (m_swShaders[i] && m_swShaders[i]->m_bindShader == shader)
			{
				m_currShader = m_swShaders[i];
				return true;
			}
		}
	}
	
	return true;
}

bool SrSoftRenderer::SetShaderConstant( uint32 slot, const float* constantStart, uint32 vec4Count )
{
	memcpy( &(m_shaderConstants[slot]), constantStart, vec4Count * sizeof(float4) );
	return true;
}

uint32 SrSoftRenderer::Tex2D( float2& texcoord, const SrTexture* texture  ) const
{
	uint32 ret = 0x00000000;
	if (texture)
	{
		if (g_context->IsFeatureEnable(eRFeature_LinearFiltering))
		{
			ret = texture->Get( texcoord, eSF_Linear );
		}
		else
		{
			ret = texture->Get( texcoord, eSF_Nearest );
		}
	}	

	return ret;
}

typedef void (*fnModuleInit)(GlobalEnvironment* pgEnv);
typedef SrSwShader* (*fnLoadShader)(const char* shaderName);

bool SrSoftRenderer::InnerInitShaders()
{
	m_swShaders.clear();

	std::string dir = "\\shader\\";
	std::string path = "\\shader\\*.swsl";
	getMediaPath(dir);
	getMediaPath(path);
	 
	WIN32_FIND_DATAA fd;
	HANDLE hff = FindFirstFileA(path.c_str(), &fd);
	BOOL bIsFind = TRUE;
	 
	while(hff && bIsFind)
	{
	 	if(fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	 	{
	 		// do not get into
	 	}
	 	else
	 	{
	 		std::string fullpath = dir + fd.cFileName;
	 
	 		// load dll shaders
	 		HMODULE hDllHandle = 0;
	 		hDllHandle = LoadLibraryA( fullpath.c_str() );
	 		if (hDllHandle)
	 		{
				fnModuleInit fnCount = (fnModuleInit)(GetProcAddress( hDllHandle, "ModuleInit" ));
	 			fnLoadShader fnLoad = (fnLoadShader)(GetProcAddress( hDllHandle, "LoadShader" ));
	 
				fnCount(gEnv);

				// add other shaders
				SrResourceManager* resMng = (SrResourceManager*)gEnv->resourceMgr;
				SrResourceLibrary::iterator it = resMng->m_shaderLibrary.begin();
				for (; it != resMng->m_shaderLibrary.end(); ++it)
				{
					SrSwShader* shader = fnLoad(it->second->getName());
					if (shader)
					{
						shader->m_bindShader = reinterpret_cast<SrShader*>(it->second);
						m_swShaders.push_back(shader);
					}
					
				}

 	 			m_swHandles.push_back(hDllHandle);
	 		}		
	 	}
	 	bIsFind = FindNextFileA(hff, &fd);
	}


	return true;
}
