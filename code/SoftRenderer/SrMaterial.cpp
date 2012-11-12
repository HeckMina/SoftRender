#include "stdafx.h"
#include "SrMaterial.h"

#include "SrObjLoader.h"

#include "SrResourceManager.h"
#include "SrDefaultMedia.h"

#include "mmgr/mmgr.h"


SrMaterial::SrMaterial( const char* name ):SrResource(name,  eRT_Material)
{
	m_shader = NULL;
	m_textures.clear();
	m_alphaTest = false;

	//LoadFromFile();
	SetShader(gEnv.resourceMgr->GetShader("PhongShader"));
}

void SrMaterial::ApplyTextures() const
{
	SrBitmapArray::const_iterator it = m_textures.begin();

	int index = 0;
	for (; it != m_textures.end(); ++it, ++index)
	{
		gEnv.renderer->SetTextureStage( *it , index );
	}
}

void SrMaterial::LoadFromFile()
{
	SetShader(gEnv.resourceMgr->GetShader("PhongShader"));

	SrObjLoader loader;

	// load file in memory
	SrMemFile matFile;
	std::string realname(m_name);
	getMediaPath(realname);
	matFile.Open( realname.c_str() );
// 	if (matFile.IsOpen())
// 	{
// 		loader.LoadMaterialFromMTL( matFile.Data() , *this );
// 	}
// 	else
// 	{
// 		char msg[250];
// 		sprintf_s(msg, "文件[ %s ]未找到！使用默认资源。", realname.c_str());
// 		MessageBox(NULL, msg, "SrResourceManager", MB_OK );
// 
// 		loader.LoadMaterialFromMTL( gEnv.resourceMgr->getDefaultMediaPack()->getDefaultMtl(), *this );
// 	}
	
}
