#include "StdAfx.h"
#include "SrHwD3D9Renderer.h"
#include <d3d9.h>
#include "SrTexture.h"
#include "SrMaterial.h"
#include "SrRasterizer.h"
#include "SrHwTextFlusher.h"
#include "SrProfiler.h"
#include "SrCamera.h"
#include "SrHwRenderTexture.h"
#include "SrHwShader.h"
#include "SrShader.h"
#include "SrResourceManager.h"


const D3DVERTEXELEMENT9 g_defaultDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	D3DDECL_END()
};

const D3DVERTEXELEMENT9 g_skinDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
	{ 0, 64, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
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

const D3DVERTEXELEMENT9 g_lineDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

SrHwD3D9Renderer::SrHwD3D9Renderer(void):IRenderer(eRt_HardwareD3D9)
{
	m_DSSize = eDSS_Full;
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
	d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// Create the D3DDevice
	if( FAILED( m_d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &m_hwDevice ) ) )
	{
		MessageBox(hWnd,"无法创建DX设备","",MB_OK);
		return false;
	}
	GtLog("[D3D9 Hw Renderer] Device Created.");

	m_hwShaders.assign(eInS_Count, NULL);
	m_hwShaders[eInS_line] = new SrHwShader("line", NULL );
	m_hwShaders[eInS_dof] = new SrHwShader("dof", NULL );
	m_hwShaders[eInS_jitaa] = new SrHwShader("jitaa", NULL );
	m_hwShaders[eInS_blur] = new SrHwShader("blur", NULL );

	// add other shaders
	SrResourceManager* resMng = (SrResourceManager*)gEnv->resourceMgr;
	SrResourceLibrary::iterator it = resMng->m_shaderLibrary.begin();
	for (; it != resMng->m_shaderLibrary.end(); ++it)
	{
		SrHwShader* hwShader = new SrHwShader( it->second->getName(), reinterpret_cast<SrShader*>(it->second) );	
		m_hwShaders.push_back(hwShader);
	}

	for (uint32 i=0; i < m_hwShaders.size(); ++i)
	{
		if (m_hwShaders[i])
		{
			m_hwShaders[i]->Init(m_hwDevice);
		}		
	}

	// Create Internal RenderTexture
	m_hwRTs.assign(eRtt_Count, NULL);
	m_hwRTs[eRtt_Backbuffer0] = new SrHwRenderTexture("backbuffer0", width, height, eHwRttype_A8R8G8B8 );
	m_hwRTs[eRtt_Backbuffer1] = new SrHwRenderTexture("backbuffer1", width, height, eHwRttype_A8R8G8B8 );
	m_hwRTs[eRtt_BackbufferBlur] = new SrHwRenderTexture("backbufferBlur", width, height, eHwRttype_A8R8G8B8 );

	m_hwRTs[eRtt_BackbufferHalf0] = new SrHwRenderTexture("backbufferHalf0", width / 2, height / 2, eHwRttype_A8R8G8B8 );
	m_hwRTs[eRtt_BackbufferHalf1] = new SrHwRenderTexture("backbufferHalf1", width / 2, height / 2, eHwRttype_A8R8G8B8 );

	m_hwRTs[eRtt_Depth] = new SrHwRenderTexture("depth", width, height, eHwRttype_R32F );

	for (uint32 i=0; i < m_hwRTs.size(); ++i)
	{
		if (m_hwRTs[i])
		{
			m_hwRTs[i]->Init(m_hwDevice);
		}		
	}

	// DepthStencil Surf
	m_hwDevice->CreateDepthStencilSurface( width, height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &m_depthStencil, NULL );
	m_hwDevice->SetDepthStencilSurface(m_depthStencil);
	m_hwDevice->CreateDepthStencilSurface( width / 2, height / 2, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &m_depthStencilHalf, NULL );
	// Get backBuffrt
	m_hwDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_backBuffer);

	
	m_hwDevice->CreateVertexDeclaration( g_defaultDecl, &m_defaultVertexDecl);
	m_hwDevice->CreateVertexDeclaration( g_skinDecl, &m_skinVertexDecl);

	m_hwDevice->CreateVertexDeclaration( g_rhzDecl, &m_rhzVertexDecl);
	m_hwDevice->CreateVertexDeclaration( g_lineDecl, &m_lineVertexDecl);


	// 关闭Z BUFFER
	m_hwDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_hwDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	m_hwDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	
// 	m_textFlusher = new SrHwTextFlusher;
// 	m_textFlusher->Init(m_hwDevice);
	
	m_gpuTimers.assign(eHt_Count, gkGpuTimer());
	
	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].init( m_hwDevice );
	}


}

bool SrHwD3D9Renderer::InnerShutdownRenderer()
{
	GtLogInfo("[D3D9 Hw Renderer] Shutting Down.");

	m_rhzVertexDecl->Release();
	m_defaultVertexDecl->Release();
	m_lineVertexDecl->Release();
	m_skinVertexDecl->Release();

	m_depthStencil->Release();

	for (uint32 i=0; i < m_hwShaders.size(); ++i)
	{
		if (m_hwShaders[i])
		{
			m_hwShaders[i]->Shutdown();
			delete m_hwShaders[i];
		}		
	}

	for (uint32 i=0; i < m_hwRTs.size(); ++i)
	{
		if (m_hwRTs[i])
		{
			m_hwRTs[i]->Shutdown();
			delete m_hwRTs[i];
		}
	}

	// clear hw textures
	for (uint32 i=0; i < m_hwTextures.size(); ++i)
	{
		static_cast<IDirect3DTexture9*>(m_hwTextures[i])->Release();
		GtLog("[D3D9 Hw Renderer] Release Binded Texture[0x%x]", m_hwTextures[i]);
	}

	for (uint32 i=0; i < m_bindVBs.size(); ++i)
	{
		m_bindVBs[i]->Release();
		GtLog("[D3D9 Hw Renderer] Release Binded VB[0x%x]", m_bindVBs[i]);
	}

	for (uint32 i=0; i < m_bindIBs.size(); ++i)
	{
		m_bindIBs[i]->Release();
		GtLog("[D3D9 Hw Renderer] Release Binded IB[0x%x]", m_bindIBs[i]);
	}
	

	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].destroy();
	}

// 	m_textFlusher->Destroy();
// 	delete m_textFlusher;

	m_backBuffer->Release();

	if( m_hwDevice != NULL )
		m_hwDevice->Release();

	if( m_d3d9 != NULL )
		m_d3d9->Release();

	GtLogInfo("[D3D9 Hw Renderer] Device Destroyed.");

	return true;
}

bool SrHwD3D9Renderer::Resize( uint32 width, uint32 height )
{
	throw std::exception("The method or operation is not implemented.");
}

void SrHwD3D9Renderer::BeginFrame()
{
	m_frameCount++;
	gEnv->profiler->setBegin(ePe_FlushTime);
	m_hwDevice->BeginScene();

	// if jit-AA, set RenderTarget
	if ( g_context->IsFeatureEnable(eRFeature_JitAA) )
	{
		if (getFrameCount() % 2 == 0)
		{
			PushRenderTarget(0, m_hwRTs[eRtt_Backbuffer0]);
		}
		else
		{
			PushRenderTarget(0, m_hwRTs[eRtt_Backbuffer1]);
		}
	}
	else
	{
		m_hwDevice->SetRenderTarget(0, m_backBuffer);
	}

	PushRenderTarget(1, m_hwRTs[eRtt_Depth]);	
}

void SrHwD3D9Renderer::EndFrame()
{
	// POST-PROCESS
	m_hwDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_hwDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_hwDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// draw line
	if (! m_drawlines.empty() )
	{
		SrCamera* cam = gEnv->sceneMgr->GetCamera("cam0");
		if (cam)
		{
			SetHwShader( m_hwShaders[eInS_line] );	
			m_hwDevice->SetVertexDeclaration(m_lineVertexDecl);

			// SetShader
			SetMatrix(eMd_WorldViewProj, cam->getViewProjMatrix());
			m_hwDevice->SetVertexShaderConstantF(0, &(m_matrixStack[eMd_WorldViewProj].m00),4);

			m_hwDevice->DrawPrimitiveUP(D3DPT_LINELIST, m_drawlines.size() / 2, &(m_drawlines[0]), sizeof(float3));
		}

		m_drawlines.clear();
	}

	PopRenderTarget(1);



	// DOF
	RP_ProcessDOF();

	// JIT-AA
	if ( g_context->IsFeatureEnable(eRFeature_JitAA) )
	{
		// jit aa
		PopRenderTarget(0);
		SetHwShader( m_hwShaders[eInS_jitaa] );	

		// set texture
		m_hwRTs[eRtt_Backbuffer0]->Apply(0,0);
		m_hwRTs[eRtt_Backbuffer1]->Apply(1,0);

		DrawScreenQuad();
	}

	m_hwDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_hwDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_hwDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	
	

	FlushText();
	m_textLines.clear();

	m_hwDevice->EndScene();

	m_hwDevice->Present(NULL, NULL, NULL, NULL);

	for (uint32 i=0; i < m_gpuTimers.size(); ++i)
	{
		m_gpuTimers[i].update();
	}

	gEnv->profiler->setEnd(ePe_FlushTime);


}

bool SrHwD3D9Renderer::HwClear()
{
	//m_hwDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,50,50,50), 1, 0);
	m_hwDevice->Clear(2, NULL, D3DCLEAR_TARGET, 0, 1, 0);
	m_hwDevice->Clear(1, NULL, D3DCLEAR_ZBUFFER, 0, 1, 0);
	IDirect3DSurface9* colorBuffer;
	m_hwDevice->GetRenderTarget(0, &colorBuffer);

	m_hwDevice->ColorFill(colorBuffer, NULL, D3DCOLOR_ARGB(255,0,0,0));

	colorBuffer->Release();

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
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	}
	else
	{
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
		m_hwDevice->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	}

	m_hwDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_hwDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

	if (texture->m_userData)
	{
		m_hwDevice->SetTexture(stage, static_cast<IDirect3DTexture9*>(texture->m_userData));
	}
	else
	{
		IDirect3DTexture9** tex = reinterpret_cast<IDirect3DTexture9**>(&(texture->m_userData));

		m_hwDevice->CreateTexture( texture->getWidth(), texture->getHeight(), 0,
			D3DUSAGE_AUTOGENMIPMAP,
			D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED,
			tex,
			NULL  );

		D3DLOCKED_RECT rect;
		if( SUCCEEDED( (*tex)->LockRect(0, &rect, NULL, NULL) )  )
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

			(*tex)->SetAutoGenFilterType(D3DTEXF_ANISOTROPIC);
			(*tex)->GenerateMipSubLevels();
			

			m_hwTextures.push_back( static_cast<IDirect3DTexture9*>(texture->m_userData) );
			GtLog("[D3D9 Hw Renderer] Texture[%s] -> Hw Texture[0x%x] Binded. Automipmaped.", texture->getName(), texture->m_userData);
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
	float time = gEnv->timer->getRealTime();

	float4 campos_w = float4(m_matrixStack[eMd_ViewInverse].GetTranslate(), g_context->viewport.f );
	float3 lightDir = gEnv->sceneMgr->m_lightList[0]->worldPos;
	SrLight* light = gEnv->sceneMgr->m_lightList[0];
	float4 amb = gEnv->sceneMgr->GetSkyLightColor();

	m_hwDevice->SetVertexShaderConstantF(0, &(m_matrixStack[eMd_WorldViewProj].m00),4);
	m_hwDevice->SetVertexShaderConstantF(4, &(m_matrixStack[eMd_World].m00),4);
	m_hwDevice->SetVertexShaderConstantF(8, &(campos_w.x),1);

	campos_w.w = primitive->material->m_alphaTest;
	m_hwDevice->SetPixelShaderConstantF(0, &(campos_w.x), 1 );
	m_hwDevice->SetPixelShaderConstantF(1, &(lightDir.x), 1 );
	m_hwDevice->SetPixelShaderConstantF(2, &(amb.x), 1 );
 	m_hwDevice->SetPixelShaderConstantF(3, &(light->diffuseColor.x), 1 );
 	m_hwDevice->SetPixelShaderConstantF(4, &(light->specularColor.x), 1 );

	// When Draw, Create Hw Buffer for it
	if (!primitive->vb->userData)
	{
		m_hwDevice->CreateVertexBuffer( primitive->vb->elementSize * primitive->vb->elementCount, 
			D3DUSAGE_WRITEONLY, 
			NULL, 
			D3DPOOL_DEFAULT, 
			(IDirect3DVertexBuffer9**)(&(primitive->vb->userData)), NULL);

		UpdateVertexBuffer(primitive->vb);

		GtLog("[D3D9 Hw Renderer] Hw VertexBuffer[0x%x] Binded.", primitive->vb);
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

		GtLog("[D3D9 Hw Renderer] Hw IndexBuffer[0x%x] Binded.", primitive->ib);
	}

	m_hwDevice->SetIndices( (IDirect3DIndexBuffer9*)(primitive->ib->userData) );
	m_hwDevice->SetStreamSource( 0, (IDirect3DVertexBuffer9*)(primitive->vb->userData), 0, primitive->vb->elementSize );

	if (primitive->material->m_alphaBlend)
	{
		m_hwDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_hwDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_hwDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_hwDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		m_hwDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, primitive->vb->elementCount, 0, primitive->ib->count / 3);

		m_hwDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_hwDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}
	else
	{
		m_hwDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, primitive->vb->elementCount, 0, primitive->ib->count / 3);
	}
	

	gEnv->profiler->setIncrement(ePe_BatchCount);
	gEnv->profiler->setIncrement(ePe_TriangleCount, primitive->ib->count / 3);
	gEnv->profiler->IncreaseTime(ePe_DrawCallTime, gEnv->timer->getRealTime() - time);

	return true;
}

bool SrHwD3D9Renderer::DrawLine( const float3& from, const float3& to )
{
	m_drawlines.push_back(from);
	m_drawlines.push_back(to);

	return true;
}

void SrHwD3D9Renderer::FlushText()
{
// HW FLUSH, THAT USING FUCK D3DX
// 	m_textFlusher->Begin();
// 
// 	SrTextLines::iterator it = m_textLines.begin();
// 	for ( ; it != m_textLines.end(); ++it )
// 	{
// 		RECT rc;
// 		rc.left = it->pos.x;
// 		rc.top = it->pos.y;
// 
// 		m_textFlusher->DrawTextLine(rc, NULL, it->color, it->text.c_str(), it->size == 0 );
// 	}
// 
// 	m_textFlusher->End();


// Try HDC
	HDC hdc ;
	HRESULT res = m_backBuffer->GetDC(&hdc);
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

	m_backBuffer->ReleaseDC(hdc);
}

bool SrHwD3D9Renderer::UpdateVertexBuffer( SrVertexBuffer* target )
{
	if (target)
	{
		if (target->userData)
		{
			IDirect3DVertexBuffer9* buffer = (IDirect3DVertexBuffer9*)(target->userData);

			void* gpuData = NULL;
			if ( SUCCEEDED( buffer->Lock( 0, target->elementSize * target->elementCount, &gpuData, NULL ) ) )
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
			if ( SUCCEEDED( buffer->Lock( 0, target->count * sizeof(uint32), &gpuData, NULL ) ) )
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

void SrHwD3D9Renderer::SetVertexShaderConstants( uint32 startIdx, const float* data, uint32 vec4Count )
{
	m_hwDevice->SetVertexShaderConstantF( startIdx, data, vec4Count);
}

void SrHwD3D9Renderer::DrawScreenQuad( SrTexture* texture )
{
	int width = g_context->width;
	int height = g_context->height;
	if (texture)
	{
		width = texture->getWidth();
		height = texture->getHeight();
	}

	m_hwDevice->SetVertexDeclaration( m_rhzVertexDecl );

	SrRendVertex quadVertex[4];

	quadVertex[0].pos = float4(-0.5f,							-0.5f,								1.0f, 1.0f);
	quadVertex[1].pos = float4(-0.5f,							height - 0.5f,						1.0f, 1.0f);
	quadVertex[2].pos = float4( -0.5f + width,					-0.5f,								1.0f, 1.0f);
	quadVertex[3].pos = float4( -0.5f + width,					height - 0.5f,						1.0f, 1.0f);

	quadVertex[0].channel1 = float4(0, 0, 1, 0);
	quadVertex[1].channel1 = float4(0, 1, 1, 0);
	quadVertex[2].channel1 = float4(1, 0, 1, 0);
	quadVertex[3].channel1 = float4(1, 1, 1, 0);

	SrCamera* cam = gEnv->sceneMgr->GetCamera("cam0");
	if (cam)
	{
		quadVertex[0].channel2 = float4(cam->m_farClipVertex[0], 1);
		quadVertex[1].channel2 = float4(cam->m_farClipVertex[1], 1);
		quadVertex[2].channel2 = float4(cam->m_farClipVertex[2], 1);
		quadVertex[3].channel2 = float4(cam->m_farClipVertex[3], 1);
	}

	

	m_hwDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &(quadVertex[0]), sizeof(SrRendVertex));
}

void SrHwD3D9Renderer::SetRenderTarget( uint8 index, SrHwRenderTexture* texture )
{
	if (index == 0)
	{
		EDSSize newSize = eDSS_Full;
		if ( texture->getWidth() == g_context->width / 2 )
		{
			newSize = eDSS_Half;
		}
		else if ( texture->getWidth() == g_context->width / 4 )
		{
			newSize = eDSS_Quad;
		}


		if (newSize != m_DSSize)
		{
			m_DSSize = newSize;

			switch ( m_DSSize)
			{
			case eDSS_Full:
				m_hwDevice->SetDepthStencilSurface( m_depthStencil );
				break;
			case eDSS_Half:
				m_hwDevice->SetDepthStencilSurface( m_depthStencilHalf );
				break;
			}
		}
	}

	IDirect3DSurface9* surf;
	((IDirect3DTexture9*)(texture->m_userData))->GetSurfaceLevel(0, &surf);	
	m_hwDevice->SetRenderTarget( index, surf );
	surf->Release();	
}

void SrHwD3D9Renderer::PushRenderTarget( uint8 index, SrHwRenderTexture* texture )
{
	if (texture)
	{
		m_RTStack[index].push(texture);

		SetRenderTarget(index, texture);
	}
}

void SrHwD3D9Renderer::PopRenderTarget( uint8 index )
{
	if (m_RTStack[index].empty())
	{

	}
	else
	{
		m_RTStack[index].pop();
	}

	if (m_RTStack[index].empty())
	{
		if (index == 0)
		{
			m_hwDevice->SetRenderTarget(0, m_backBuffer);
			m_hwDevice->SetDepthStencilSurface( m_depthStencil );
		}
		else
		{
			m_hwDevice->SetRenderTarget(index, NULL);
		}		
	}
	else
	{
		SetRenderTarget( index, m_RTStack[index].top() );
	}
}

void SrHwD3D9Renderer::StretchRT2TEX( uint8 index, SrHwRenderTexture* texture )
{
	IDirect3DSurface9* source;
	IDirect3DSurface9* target;
	m_hwDevice->GetRenderTarget(index, &source);
	((IDirect3DTexture9*)(texture->m_userData))->GetSurfaceLevel(0, &target);

	m_hwDevice->StretchRect( source, NULL, target, NULL, D3DTEXF_NONE );

	source->Release();
	target->Release();
}

void SrHwD3D9Renderer::StretchTEX2TEX( SrHwRenderTexture* sourcetex, SrHwRenderTexture* targettex )
{
	IDirect3DSurface9* source;
	IDirect3DSurface9* target;
	((IDirect3DTexture9*)(sourcetex->m_userData))->GetSurfaceLevel(0, &source);
	((IDirect3DTexture9*)(targettex->m_userData))->GetSurfaceLevel(0, &target);

	m_hwDevice->StretchRect( source, NULL, target, NULL, D3DTEXF_NONE );

	source->Release();
	target->Release();
}

void SrHwD3D9Renderer::RP_ProcessDOF()
{
// 	if (g_context->dofDist == 0)
// 	{
// 		return;
// 	}

	StretchRT2TEX( 0, m_hwRTs[eRtt_BackbufferBlur] );

	FX_GaussisanBlur(m_hwRTs[eRtt_BackbufferBlur], 2.5f, 1.0f, 2);

	// dof combine
	// set param
	float dof = 15.0f;

	float4 campos = float4(m_matrixStack[eMd_ViewInverse].GetTranslate(), 1.f);
	m_hwDevice->SetPixelShaderConstantF(0, &campos.x, 1);

	m_hwDevice->SetPixelShaderConstantF(1, &dof, 1);

	// set shader
	SetHwShader( m_hwShaders[eInS_dof] );
	m_hwDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_hwDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_hwDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_hwRTs[eRtt_BackbufferBlur]->Apply(0,0);

	//const SrTexture* tex = gEnv->resourceMgr->LoadTexture( "default_d");
	//tex->Apply(0,0);
	m_hwRTs[eRtt_Depth]->Apply(1,0);

	DrawScreenQuad();

	m_hwDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void SrHwD3D9Renderer::FX_GaussisanBlur( SrHwRenderTexture* texture, float disort, float scale, int iterate )
{
	// blur first
	float fDistribution = disort;
	float fScale = scale;
	float s1 = 1.f / (g_context->width * 0.5f);
	float t1 = 1.f / (g_context->height * 0.5f);
	float4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );

	// Horizontal/Vertical pass params
	const int nSamples = 16;
	int nHalfSamples = (nSamples>>1);

	float4 pHParams[32], pVParams[32], pWeightsPS[32];
	float pWeights[32], fWeightSum = 0;

	memset( pWeights,0,sizeof(pWeights) );

	int s;
	for(s = 0; s<nSamples; ++s)
	{
		if(fDistribution != 0.0f)
			pWeights[s] = GaussianDistribution1D(s - nHalfSamples, fDistribution);      
		else
			pWeights[s] = 0.0f;
		fWeightSum += pWeights[s];
	}

	// normalize weights
	for(s = 0; s < nSamples; ++s)
	{
		pWeights[s] /= fWeightSum;  
	}

	// set bilinear offsets
	for(s = 0; s < nHalfSamples; ++s)
	{
		float off_a = pWeights[s*2];
		float off_b = ( (s*2+1) <= nSamples-1 )? pWeights[s*2+1] : 0;   
		float a_plus_b = (off_a + off_b);
		if (a_plus_b == 0)
			a_plus_b = 1.0f;
		float offset = off_b / a_plus_b;

		pWeights[s] = off_a + off_b;
		pWeights[s] *= fScale ;
		pWeightsPS[s] = vWhite * pWeights[s];

		float fCurrOffset = (float) s*2 + offset - nHalfSamples;
		pHParams[s] = float4(s1 * fCurrOffset , 0, 0, 0);  
		pVParams[s] = float4(0, t1 * fCurrOffset , 0, 0);       
	}

	SetHwShader( m_hwShaders[eInS_blur] );

	StretchTEX2TEX( texture, m_hwRTs[eRtt_BackbufferHalf0] );
	PushRenderTarget(0, m_hwRTs[eRtt_BackbufferHalf1]);

	m_hwRTs[eRtt_BackbufferHalf0]->Apply(0,0);

	m_hwDevice->SetPixelShaderConstantF(0, &(pHParams[0].x), 8);
	m_hwDevice->SetPixelShaderConstantF(8, &(pWeightsPS[0].x), 8);

	DrawScreenQuad( m_hwRTs[eRtt_BackbufferHalf1] );
	PopRenderTarget(0);	

	PushRenderTarget(0, m_hwRTs[eRtt_BackbufferHalf0]);
	m_hwRTs[eRtt_BackbufferHalf1]->Apply(0,0);

	m_hwDevice->SetPixelShaderConstantF(0, &(pVParams[0].x), 8);
	m_hwDevice->SetPixelShaderConstantF(8, &(pWeightsPS[0].x), 8);

	DrawScreenQuad( m_hwRTs[eRtt_BackbufferHalf0] );
	PopRenderTarget(0);

	//////////////////////////////////////////////////////////////////////////
	// iterate 
	for (int i=0; i < iterate - 1; ++i)
	{
		PushRenderTarget(0, m_hwRTs[eRtt_BackbufferHalf1]);

		m_hwRTs[eRtt_BackbufferHalf0]->Apply(0,0);

		m_hwDevice->SetPixelShaderConstantF(0, &(pHParams[0].x), 8);
		m_hwDevice->SetPixelShaderConstantF(8, &(pWeightsPS[0].x), 8);

		DrawScreenQuad( m_hwRTs[eRtt_BackbufferHalf1] );
		PopRenderTarget(0);	

		PushRenderTarget(0, m_hwRTs[eRtt_BackbufferHalf0]);
		m_hwRTs[eRtt_BackbufferHalf1]->Apply(0,0);

		m_hwDevice->SetPixelShaderConstantF(0, &(pVParams[0].x), 8);
		m_hwDevice->SetPixelShaderConstantF(8, &(pWeightsPS[0].x), 8);

		DrawScreenQuad( m_hwRTs[eRtt_BackbufferHalf0] );
		PopRenderTarget(0);
	}

	StretchTEX2TEX( m_hwRTs[eRtt_BackbufferHalf0], texture );
}

bool SrHwD3D9Renderer::SetHwShader( SrHwShader* shader )
{
	if (shader)
	{
		if (shader->m_vs)
		{
			m_hwDevice->SetVertexShader( shader->m_vs );
		}

		if (shader->m_ps)
		{
			m_hwDevice->SetPixelShader( shader->m_ps );
		}
	}
	
	return true;
}

bool SrHwD3D9Renderer::SetShader( const SrShader* shader )
{
	if (shader)
	{
		for ( uint32 i=0; i < m_hwShaders.size(); ++i )
		{
			if (m_hwShaders[i] && m_hwShaders[i]->m_bindShader == shader)
			{
				switch( shader->m_vertDecl )
				{
				case eVd_F4F4F4:
					m_hwDevice->SetVertexDeclaration( m_defaultVertexDecl );
					break;
				case eVd_F4F4F4F4U4:
					m_hwDevice->SetVertexDeclaration( m_skinVertexDecl );
					break;
				}
				SetHwShader( m_hwShaders[i] );
				return true;
			}
		}
	}

	return false;
}

bool SrHwD3D9Renderer::SetShaderConstant( uint32 slot, const float* constantStart, uint32 vec4Count )
{
	// set shader constant judge by slot
	if (slot < eSC_PS0)
	{
		// vs

		m_hwDevice->SetVertexShaderConstantF( slot, constantStart, vec4Count );
	}
	else
	{
		// ps

		m_hwDevice->SetPixelShaderConstantF( slot - eSC_PS0, constantStart, vec4Count);
	}

	return true;
}

uint32 SrHwD3D9Renderer::Tex2D( float2& texcoord, const SrTexture* texture ) const
{
	return 0;
}

bool SrHwD3D9Renderer::UpdateShaderConstantsPerFrame()
{
	return true;
}


