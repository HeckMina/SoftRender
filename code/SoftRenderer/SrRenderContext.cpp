/**
  @file SrRenderContext.cpp
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#include "StdAfx.h"
#include "SrRenderContext.h"
#include "SrFragmentBuffer.h"

#include "mmgr/mmgr.h"


SrRendContext::SrRendContext( int w, int h, int obpp )
{
	memset(this, 0, sizeof(SrRendContext));
	width = w;
	height = h;
	bpp = obpp / 8;
	viewport = SrViewport(0.f,0.f,(float)width,(float)height,1.f,1000.f);

	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo); 

	processorNum = siSysInfo.dwNumberOfProcessors;
}
