/**
  @file SrBitmap.cpp
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#include "StdAfx.h"
#include "SrBitmap.h"
#include "atlimage.h"
#include "SrProfiler.h"

#include "mmgr/mmgr.h"


SrBitmap::SrBitmap(const char* filename):SrTexture(filename)
{
	m_image = new ATL::CImage;

	std::string realpath(filename);
	getMediaPath(realpath);

	HRESULT res = m_image->Load(realpath.c_str());
	if (res!=S_OK)
	{
		m_image->ReleaseDC();
		m_image->Destroy();
		delete m_image;
		m_image =0;
		m_data = 0;
		return;
	}
	m_texType = eBt_file;
	m_width = m_image->GetWidth();
	m_height = m_image->GetHeight();
	m_bpp = m_image->GetBPP() / 8;
	m_pitch = m_image->GetPitch();
	m_data = (uint8*)m_image->GetBits();
}

SrBitmap::~SrBitmap(void)
{
	if (m_image)
	{
		m_image->ReleaseDC();
		m_image->Destroy();
		delete m_image;
	}
}

