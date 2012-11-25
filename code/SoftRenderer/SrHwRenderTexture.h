/**
  @file SrHwRenderTexture.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrHwRenderTexture_h__
#define SrHwRenderTexture_h__
#include "SrTexture.h"

enum EHwRTType
{
	eHwRttype_R32F,
	eHwRttype_A8R8G8B8,
};

class SrHwRenderTexture : public SrTexture
{
public:
	SrHwRenderTexture(const char* name, int width, int height, EHwRTType type);
	~SrHwRenderTexture(void);

	void Init(struct IDirect3DDevice9* device);
	void Shutdown();

	EHwRTType m_hwRTType;
};

#endif // SrHwRenderTexture_h__




