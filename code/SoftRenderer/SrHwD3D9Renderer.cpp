#include "StdAfx.h"
#include "SrHwD3D9Renderer.h"
#include <d3d9.h>
#include "SrTexture.h"
#include "SrMaterial.h"
#include "SrRasterizer.h"
#include "SrHwTextFlusher.h"
#include "SrProfiler.h"


const D3DVERTEXELEMENT9 g_defaultDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
	{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

const D3DVERTEXELEMENT9 g_rhzDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
	D3DDECL_END()
};

SrHwD3D9Renderer::SrHwD3D9Renderer(void):IRenderer(eRt_HardwareD3D9)
{
	m_defaultVS = NULL;
	m_defaultPS = NULL;
	m_rhzVS = NULL;
}

SrHwD3D9Renderer::~SrHwD3D9Renderer(void)
{
}

bool SrHwD3D9Renderer::InnerInitRenderer( HWND hWnd, int width, int height, int bpp )
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
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	//d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// Create the D3DDevice
	if( FAILED( m_d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &m_hwDevice ) ) )
	{
		MessageBox(hWnd,"无法创建DX设备","",MB_OK);
		return false;
	}

 	m_hwDevice->CreateTexture( width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_backBuffer0, NULL );
 	m_hwDevice->CreateTexture( width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_backBuffer1, NULL );

	m_hwDevice->CreateDepthStencilSurface( width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_depthStencil, NULL );
	m_hwDevice->SetDepthStencilSurface(m_depthStencil);

	m_hwDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_backBuffer);

	// using default effectr
	SrMemFile file;
	std::string path("media\\shader\\default_vs.o");
	getMediaPath(path);
	file.Open( path.c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(m_hwDevice->CreateVertexShader( (const DWORD*)(file.Data()), &m_defaultVS )) )
		{
			OutputDebugString( "default VS Creation failed." );
		}
	}
	file.Close();

	path = "media\\shader\\default_ps.o";
	getMediaPath(path);
	file.Open( path.c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(m_hwDevice->CreatePixelShader( (const DWORD*)(file.Data()), &m_defaultPS )) )
		{
			OutputDebugString( "default PS Creation failed." );
		}
	}
	file.Close();

	path = "media\\shader\\jitaa_ps.o";
	getMediaPath(path);
	file.Open( path.c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(m_hwDevice->CreatePixelShader( (const DWORD*)(file.Data()), &m_jitaaPS )) )
		{
			OutputDebugString( "default PS Creation failed." );
		}
	}
	file.Close();
	
	path = "media\\shader\\rhz_vs.o";
	getMediaPath(path);
	file.Open( path.c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(m_hwDevice->CreateVertexShader( (const DWORD*)(file.Data()), &m_rhzVS )) )
		{
			OutputDebugString( "default PS Creation failed." );
		}
	}
	file.Close();
	
	m_hwDevice->CreateVertexDeclaration( g_defaultDecl, &m_defaultVertexDecl);
	m_hwDevice->CreateVertexDeclaration( g_rhzDecl, &m_rhzVertexDecl);

	// 打开Z BUFFER
	m_hwDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_hwDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	
	// text Flusher建立
	m_textFlusher = new SrHwTextFlusher;
	m_textFlusher->Init(m_hwDevice);
	
	// gpuTimer建立
	m_gpuTimers.assign(eHt_Count, gkGpuTimer());
	
	// 初始化gpuTimer
	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].init( m_hwDevice );
	}


}

bool SrHwD3D9Renderer::InnerShutdownRenderer()
{
	// destroy shaders
	m_defaultVS->Release();
	m_defaultPS->Release();

	m_rhzVS->Release();
	m_jitaaPS->Release();

	m_rhzVertexDecl->Release();
	m_defaultVertexDecl->Release();

	m_depthStencil->Release();

	m_backBuffer0->Release();
	m_backBuffer1->Release();

	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].destroy();
	}

	m_textFlusher->Destroy();
	delete m_textFlusher;

	// clear hw textures
	for (uint32 i=0; i < m_hwTextures.size(); ++i)
	{
		static_cast<IDirect3DTexture9*>(m_hwTextures[i])->Release();
	}

	if( m_hwDevice != NULL )
		m_hwDevice->Release();

	if( m_d3d9 != NULL )
		m_d3d9->Release();

	return true;
}

bool SrHwD3D9Renderer::Resize( uint32 width, uint32 height )
{
	throw std::exception("The method or operation is not implemented.");
}

void SrHwD3D9Renderer::BeginFrame()
{
	m_frameCount++;
	gEnv.profiler->setBegin(ePe_FlushTime);
	m_hwDevice->BeginScene();


	// if jit-AA, set RenderTarget
	if ( g_context->IsFeatureEnable(eRFeature_JitAA) )
	{
		IDirect3DSurface9* backBuffer0;

		if (getFrameCount() % 2 == 0)
		{
			m_backBuffer0->GetSurfaceLevel(0, &backBuffer0);
		}
		else
		{
			m_backBuffer1->GetSurfaceLevel(0, &backBuffer0);
		}
		

		m_hwDevice->SetRenderTarget(0, backBuffer0);

		backBuffer0->Release();
	}
	else
	{
		m_hwDevice->SetRenderTarget(0, m_backBuffer);
	}
}

void SrHwD3D9Renderer::EndFrame()
{
	// do jit-AA here
	if ( g_context->IsFeatureEnable(eRFeature_JitAA) )
	{
		m_hwDevice->SetRenderTarget(0, m_backBuffer);
		
		SrRendVertex quadVertex[4];

		quadVertex[0].pos = float4(-0.5f,						-0.5f,						1.0f, 1.0f);
		quadVertex[1].pos = float4(-0.5f,						g_context->height - 0.5f,	1.0f, 1.0f);
		quadVertex[2].pos = float4( -0.5f + g_context->width,	-1.0f,						1.0f, 1.0f);
		quadVertex[3].pos = float4( -0.5f + g_context->width,	g_context->height - 0.5f,	1.0f, 1.0f);


		quadVertex[0].channel1 = float4(0, 0, 0, 0);
		quadVertex[1].channel1 = float4(0, 1, 0, 0);
		quadVertex[2].channel1 = float4(1, 0, 0, 0);
		quadVertex[3].channel1 = float4(1, 1, 0, 0);

		m_hwDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_hwDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

		m_hwDevice->SetVertexShader(m_rhzVS);
		m_hwDevice->SetPixelShader(m_jitaaPS);

		// set texture
		m_hwDevice->SetTexture(0, m_backBuffer0);
		m_hwDevice->SetTexture(1, m_backBuffer1);

		m_hwDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

		// SetVertexDeclaration
		m_hwDevice->SetVertexDeclaration( m_rhzVertexDecl );

		m_hwDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &(quadVertex[0]), sizeof(SrRendVertex));

		m_hwDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_hwDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}
	
	m_hwDevice->EndScene();

	FlushText();
	m_textLines.clear();

	m_hwDevice->Present(NULL, NULL, NULL, NULL);

	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].update();
	}

	gEnv.profiler->setEnd(ePe_FlushTime);


}

bool SrHwD3D9Renderer::HwClear()
{
	m_hwDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,SR_GREYSCALE_CLEARCOLOR,SR_GREYSCALE_CLEARCOLOR,SR_GREYSCALE_CLEARCOLOR), 1, 0);
	return true;
}

void* SrHwD3D9Renderer::getBuffer()
{
	return NULL;
}

bool SrHwD3D9Renderer::SetTextureStage( const SrTexture* texture, int stage )
{
	if ( g_context->IsFeatureEnable(eRFeature_LinearFiltering) )
	{
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	else
	{
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
	}

	if (texture->m_userData)
	{
		m_hwDevice->SetTexture(stage, static_cast<IDirect3DTexture9*>(texture->m_userData));
	}
	else
	{
		// 没有被加入，那么创建一下

		// FMT DETECTD!!
		//D3DFORMAT format = texture->getBPP() == 3? D3DFMT_R8G8B8 : D3DFMT_A8R8G8B8
		// COPY
		IDirect3DTexture9** tex = reinterpret_cast<IDirect3DTexture9**>(&(texture->m_userData));

		m_hwDevice->CreateTexture( texture->getWidth(), texture->getHeight(), 0,
			D3DUSAGE_DYNAMIC,
			D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT,
			tex,
			NULL  );
		
		D3DLOCKED_RECT rect;
		if( SUCCEEDED( (*tex)->LockRect(0, &rect, NULL, D3DLOCK_DISCARD) )  )
		{
			// copy line by line
			// if argb
			if (texture->getBPP() == 4)
			{
				for (uint32 i=0; i < texture->getHeight(); ++i)
				{
					memcpy( (uint8*)(rect.pBits) + rect.Pitch * i, texture->getBuffer() + texture->getPitch() * i, texture->getWidth() * texture->getBPP() );
				}
			}
			else
			{
				// else 
				// copy one by one, ca!
				for (uint32 i=0; i < texture->getHeight(); ++i)
				{
					for (uint32 j=0; j < texture->getWidth(); ++j)
					{
						uint8* dest = (uint8*)(rect.pBits) + rect.Pitch * i + j * 4;
						*((uint32*)dest) = uint8BGR_2_uint32(texture->getBuffer() + texture->getPitch() * i + texture->getBPP() * j);
					}
				}
			}
									
			(*tex)->UnlockRect(0);

			m_hwTextures.push_back( static_cast<IDirect3DTexture9*>(texture->m_userData) );
		}
		m_hwDevice->SetTexture(stage, reinterpret_cast<IDirect3DTexture9*>(texture->m_userData));
	}
	return true;
}

void SrHwD3D9Renderer::ClearTextureStage()
{
	for (uint32 i =0; i < 16; ++i)
	{
		m_hwDevice->SetTexture(i, NULL);
	}	
}

bool SrHwD3D9Renderer::DrawPrimitive( SrPrimitve* primitive )
{
	// dp time count
	float time = gEnv.timer->getRealTime();


	m_hwDevice->SetVertexShader(m_defaultVS);
	m_hwDevice->SetPixelShader(m_defaultPS);

	// SetShader
	m_hwDevice->SetVertexShaderConstantF(0, &(m_matrixStack[eMd_WorldViewProj].m00),4);
	m_hwDevice->SetVertexShaderConstantF(4, &(m_matrixStack[eMd_World].m00),4);
	
	float4 campos;
	campos.xyz = m_matrixStack[eMd_ViewInverse].GetTranslate();
	campos.w = 0;
	if ( primitive->material->m_alphaTest )
	{
		campos.w = 0.01f;
	}

	float3 lightDir = gEnv.sceneMgr->m_lightList[0]->worldPos;
	SrLight* light = gEnv.sceneMgr->m_lightList[0];
	float4 amb = gEnv.sceneMgr->GetSkyLightColor();

	m_hwDevice->SetPixelShaderConstantF(0, &(campos.x), 1 );
	m_hwDevice->SetPixelShaderConstantF(1, &(lightDir.x), 1 );
	m_hwDevice->SetPixelShaderConstantF(2, &(amb.x), 1 );
 	m_hwDevice->SetPixelShaderConstantF(3, &(light->diffuseColor.x), 1 );
 	m_hwDevice->SetPixelShaderConstantF(4, &(light->specularColor.x), 1 );

//	SrCbuffer_General* cBuffer = reinterpret_cast<SrCbuffer_General*>(&(primitive->material->m_cbuffer));

// 	m_hwDevice->SetPixelShaderConstantF(5, &(), 1 );
// 	m_hwDevice->SetPixelShaderConstantF(6, &(cBuffer->spcColor.x), 1 );
// 	m_hwDevice->SetPixelShaderConstantF(7, &(cBuffer->glossness), 1 );

	// SetTexture
	primitive->material->ApplyTextures();

	// SetVertexDeclaration
	m_hwDevice->SetVertexDeclaration( m_defaultVertexDecl );

	// When Draw, Create Hw Buffer for it
	if (!primitive->vb->userData)
	{
		m_hwDevice->CreateVertexBuffer( primitive->vb->elementSize * primitive->vb->elementCount, 
			D3DUSAGE_WRITEONLY, 
			NULL, 
			D3DPOOL_DEFAULT, 
			(IDirect3DVertexBuffer9**)(&(primitive->vb->userData)), NULL);

		UpdateVertexBuffer(primitive->vb);
	}

	if (!primitive->ib->userData)
	{
		m_hwDevice->CreateIndexBuffer( primitive->ib->count * sizeof(uint32), 
			D3DUSAGE_WRITEONLY, 
			D3DFMT_INDEX32, 
			D3DPOOL_DEFAULT,
			(IDirect3DIndexBuffer9**)(&(primitive->ib->userData)),
			NULL );

		UpdateIndexBuffer(primitive->ib);
	}

	m_hwDevice->SetIndices( (IDirect3DIndexBuffer9*)(primitive->ib->userData) );
	m_hwDevice->SetStreamSource( 0, (IDirect3DVertexBuffer9*)(primitive->vb->userData), 0, primitive->vb->elementSize );
	m_hwDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, primitive->vb->elementCount, 0, primitive->ib->count / 3);


	gEnv.profiler->setIncrement(ePe_BatchCount);
	gEnv.profiler->setIncrement(ePe_TriangleCount, primitive->ib->count / 3);
	gEnv.profiler->IncreaseTime(ePe_DrawCallTime, gEnv.timer->getRealTime() - time);

	return true;
}

bool SrHwD3D9Renderer::DrawLine( const float3& from, const float3& to )
{
	return true;
}

void SrHwD3D9Renderer::FlushText()
{
	m_textFlusher->Begin();

	SrTextLines::iterator it = m_textLines.begin();
	for ( ; it != m_textLines.end(); ++it )
	{
		RECT rc;
		rc.left = it->pos.x;
		rc.top = it->pos.y;

		m_textFlusher->DrawTextLine(rc, NULL, it->color, it->text.c_str(), it->size == 0 );
	}

	m_textFlusher->End();
}

SrVertexBuffer* SrHwD3D9Renderer::AllocateVertexBuffer( uint32 elementSize, uint32 count, bool fastmode /*= false */ )
{
	SrVertexBuffer* created = IRenderer::AllocateVertexBuffer(elementSize, count, fastmode);
	return created;
}

bool SrHwD3D9Renderer::DeleteVertexBuffer( SrVertexBuffer* target )
{
	for (uint32 i=0; i < m_vertexBuffers.size(); ++i)
	{
		if( m_vertexBuffers[i] == target )
		{
			_mm_free( m_vertexBuffers[i]->data );
			
			if (target->userData)
			{
				((IDirect3DVertexBuffer9*)(target->userData))->Release();
			}
			
			delete (m_vertexBuffers[i]);
			m_vertexBuffers[i] = NULL;

			// 卸载了就跳出啊！
			return true;
		}
	}

	return false;
}

SrIndexBuffer* SrHwD3D9Renderer::AllocateIndexBuffer( uint32 count )
{
	SrIndexBuffer* created = IRenderer::AllocateIndexBuffer( count );
	return created;
}

bool SrHwD3D9Renderer::DeleteIndexBuffer( SrIndexBuffer* target )
{
	for (uint32 i=0; i < m_indexBuffers.size(); ++i)
	{
		if( m_indexBuffers[i] == target )
		{
			if (m_indexBuffers[i]->data)
			{
				delete[] m_indexBuffers[i]->data;
				m_indexBuffers[i]->data = NULL;
			}

			if (target->userData)
			{
				((IDirect3DIndexBuffer9*)(target->userData))->Release();
			}

			delete m_indexBuffers[i];
			m_indexBuffers[i] = NULL;

			// 卸载了就跳出啊！
			return true;
		}
	}

	return false;
}

bool SrHwD3D9Renderer::UpdateVertexBuffer( SrVertexBuffer* target )
{
	if (target)
	{
		if (target->userData)
		{
			IDirect3DVertexBuffer9* buffer = (IDirect3DVertexBuffer9*)(target->userData);

			void* gpuData = NULL;
			if ( SUCCEEDED( buffer->Lock( 0, target->elementSize * target->elementCount, &gpuData, D3DLOCK_DISCARD ) ) )
			{
				memcpy( gpuData, target->data, target->elementSize * target->elementCount );

				buffer->Unlock();
			}			
		}
	}

	return true;
}

bool SrHwD3D9Renderer::UpdateIndexBuffer( SrIndexBuffer* target )
{
	if (target)
	{
		if (target->userData)
		{
			IDirect3DIndexBuffer9* buffer = (IDirect3DIndexBuffer9*)(target->userData);

			void* gpuData = NULL;
			if ( SUCCEEDED( buffer->Lock( 0, target->count * sizeof(uint32), &gpuData, D3DLOCK_DISCARD ) ) )
			{
				memcpy( gpuData, target->data, target->count * sizeof(uint32) );

				buffer->Unlock();
			}			
		}
	}

	return true;
}

void SrHwD3D9Renderer::SetGpuMarkStart( EHwTimerElement element )
{
	m_gpuTimers[element].start();
}

void SrHwD3D9Renderer::SetGpuMarkEnd( EHwTimerElement element )
{
	m_gpuTimers[element].stop();
}

float SrHwD3D9Renderer::GetGpuTime( EHwTimerElement element )
{
	return m_gpuTimers[element].getTime();
}

bool SrHwD3D9Renderer::SetShader( const SrShader* shader )
{
	return false;
}

bool SrHwD3D9Renderer::SetShaderConstant( EShaderConstantsSlot slot, const float* constantStart, uint32 vec4Count )
{
	return false;
}
