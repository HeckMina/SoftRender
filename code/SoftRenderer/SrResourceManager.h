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
	virtual ~SrResourceManager(void);

	virtual SrMesh*				LoadMesh(const char* filename);
	virtual const SrTexture*	LoadTexture(const char* filename, bool bump = false);
	virtual SrMaterial*			LoadMaterial(const char* filename);
	virtual SrMaterial*			CreateMaterial(const char* filename);
	virtual void				LoadMaterialLib(const char* filename);
	virtual SrShader*			GetShader(const char* name);
	virtual void				AddShader(SrShader* shader);

	virtual SrTexture*			CreateRenderTexture(const char* name, int width, int height, int bpp);
	virtual SrMaterial*			CreateManmualMaterial(const char* name);

	virtual void				InitDefaultMedia();
	virtual SrDefaultMediaPack*	getDefaultMediaPack() {return m_defaultMediaPack;}

	virtual void ReloadShaders();

private:

	void UnloadSwShaders();
	void LoadSwShaders();

	SrResourceLibrary	m_meshLibrary;
	SrResourceLibrary	m_textureLibrary;
	SrResourceLibrary	m_materialLibrary;
	SrResourceLibrary	m_shaderLibrary;

	SrDefaultMediaPack* m_defaultMediaPack;

	SrSwShaderHandles m_handles;
};

#endif // SrResourceManager_h__



