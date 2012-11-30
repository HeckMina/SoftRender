#include "StdAfx.h"
#include "SrHwRenderTexture.h"
#include <d3d9.h>

SrHwRenderTexture::SrHwRenderTexture( const char* name, int width, int height, EHwRTType type ):SrTexture(name)
{
	m_width = width;
	m_height = height;
	
	m_data = NULL;
	m_texType = eBt_internal;

	m_hwRTType = type;
}


SrHwRenderTexture::~SrHwRenderTexture(void)
{

}

void SrHwRenderTexture::Init( struct IDirect3DDevice9* device )
{
	D3DFORMAT format = D3DFMT_A8R8G8B8;
	switch(m_hwRTType)
	{
	case eHwRttype_R32F:
		format = D3DFMT_R32F;
		break;
	case eHwRttype_A8R8G8B8:
		format = D3DFMT_A8R8G8B8;
		break;
	}


	device->CreateTexture( m_width, m_height, 0, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, (IDirect3DTexture9**)(&m_userData), NULL );
	GtLog("[D3D9 Hw Renderer] Hw RenderTexture [%s] Created. width= %d height= %d", m_name.c_str(), m_width, m_height);
}

void SrHwRenderTexture::Shutdown()
{
	if (m_userData)
	{
		((IDirect3DTexture9*)m_userData)->Release();
		GtLog("[D3D9 Hw Renderer] Hw RenderTexture [%s] Destroyed.", m_name.c_str());
	}
	m_userData = NULL;
}
