/**
  @file SrResourceManager.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrResourceManager_h__
#define SrResourceManager_h__

#include "prerequisite.h"

struct SrDefaultMediaPack;

class SrResourceManager : public IResourceManager
{
public:
	typedef std::map<std::string, SrResource*> SrResourceLibrary;
	typedef std::vector<HMODULE> SrSwShaderHandles;

public:
	SrResourceManager(void);
	~SrResourceManager(void);

	SrMesh*				LoadMesh(const char* filename);
	const SrTexture*	LoadTexture(const char* filename, bool bump = false);
	SrMaterial*			LoadMaterial(const char* filename);
	SrMaterial*			CreateMaterial(const char* filename);
	void				LoadMaterialLib(const char* filename);
	SrShader*			GetShader(const char* name);
	void				AddShader(SrShader* shader);

	SrTexture*			CreateRenderTexture(const char* name, int width, int height, int bpp);
	SrMaterial*			CreateManmualMaterial(const char* name);

	void				InitDefaultMedia();
	SrDefaultMediaPack*	getDefaultMediaPack() {return m_defaultMediaPack;}

	virtual void ReloadShaders();

private:
	SrResourceLibrary	m_meshLibrary;
	SrResourceLibrary	m_textureLibrary;
	SrResourceLibrary	m_materialLibrary;
	SrResourceLibrary	m_shaderLibrary;

	SrDefaultMediaPack* m_defaultMediaPack;

	SrSwShaderHandles m_handles;
};

#endif // SrResourceManager_h__



