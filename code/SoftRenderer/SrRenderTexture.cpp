#include "StdAfx.h"
#include "SrRenderTexture.h"
#include "mmgr/mmgr.h"

SrRenderTexture::SrRenderTexture( const char* name, int width, int height, int bpp ):SrTexture(name)
{
	m_width = width;
	m_height = height;
	m_bpp = bpp;
	m_data = NULL;
	m_texType = eBt_internal;
	m_pitch = bpp * width;
	// allocate buffer
	m_data = new uint8[width * height * bpp];
	memset(m_data, 0, width * height * bpp );
}

SrRenderTexture::~SrRenderTexture(void)
{
	if (m_data)
	{
		delete[] m_data;
	}
}
