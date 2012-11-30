/**
  @file SrHwShader.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrHwShader_h__
#define SrHwShader_h__

#include "SrResource.h"

struct IDirect3DDevice9;

class SrHwShader : public SrResource
{
public:
	SrHwShader(const char* name, SrShader* bindShader);
	~SrHwShader(void);

	void Init(IDirect3DDevice9* device);
	void Shutdown();

	struct IDirect3DVertexShader9* m_vs;
	struct IDirect3DPixelShader9* m_ps;
public:
	class SrShader* m_bindShader;
};

#endif // SrHwShader_h__


