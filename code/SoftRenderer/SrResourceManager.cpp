#include "StdAfx.h"
#include "SrResourceManager.h"
#include "SrMesh.h"
#include "SrRenderTexture.h"
#include "SrBitmap.h"
#include "SrMaterial.h"
#include "SrDefaultMedia.h"
#include "SrObjLoader.h"
#include "SrShader.h"

#include "mmgr/mmgr.h"





SrResourceManager::SrResourceManager(void)
{
	m_textureLibrary.clear();
	m_materialLibrary.clear();
	m_meshLibrary.clear();
	m_shaderLibrary.clear();
}


SrResourceManager::~SrResourceManager(void)
{
	// destroy all loaded resources

	SrResourceLibrary::iterator it = m_meshLibrary.begin();
	for (; it != m_meshLibrary.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

	it = m_textureLibrary.begin();
	for (; it != m_textureLibrary.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

	it = m_materialLibrary.begin();
	for (; it != m_materialLibrary.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

// 	it = m_shaderLibrary.begin();
// 	for (; it != m_shaderLibrary.end(); ++it)
// 	{
// 		if (it->second)
// 		{
// 			delete it->second;
// 		}
// 	}

	delete m_defaultMediaPack;
}

SrMesh* SrResourceManager::LoadMesh( const char* filename )
{
	SrMesh* ret = NULL;
	SrResourceLibrary::iterator it = m_meshLibrary.find(filename);

	if (it != m_meshLibrary.end())
	{
		ret = static_cast<SrMesh*>(it->second);
	}
	else
	{
		ret = new SrMesh(filename);
		m_meshLibrary.insert(SrResourceLibrary::value_type( filename, ret ));
	}

	return ret;	
}

const SrTexture* SrResourceManager::LoadTexture( const char* filename, bool bump )
{
	SrTexture* ret = NULL;
	SrResourceLibrary::iterator it = m_textureLibrary.find(filename);

	if (it != m_textureLibrary.end())
	{
		// 这里是可能找到内置纹理的
		ret = static_cast<SrTexture*>(it->second);
	}
	else
	{
		// 如果没有创建，去创建外部bitmap
		ret = new SrBitmap(filename);
		// 这里有可能创建不成功
		if ( !ret || !(ret->getBuffer()) )
		{
			delete ret;
			if (bump)
			{
				ret = m_defaultMediaPack->getDefaultFlatTex();
			}
			else
			{
				ret = m_defaultMediaPack->getDefaultTex();
			}
			
		}
		else
		{
			m_textureLibrary.insert(SrResourceLibrary::value_type( filename, ret ));
		}		
	}

	return ret;	
}

SrMaterial* SrResourceManager::LoadMaterial( const char* filename )
{
	SrMaterial* ret = NULL;
	SrResourceLibrary::iterator it = m_materialLibrary.find(filename);

	if (it != m_materialLibrary.end())
	{
		ret = static_cast<SrMaterial*>(it->second);
	}
	else
	{
		// 默认材质
		ret = LoadMaterial( "$srdefualt" );
	}
	return ret;	
}


SrMaterial* SrResourceManager::CreateMaterial( const char* filename )
{
	SrMaterial* ret = NULL;
	SrResourceLibrary::iterator it = m_materialLibrary.find(filename);

	if (it != m_materialLibrary.end())
	{
		ret = static_cast<SrMaterial*>(it->second);
	}
	else
	{
		ret = new SrMaterial(filename);
		m_materialLibrary.insert(SrResourceLibrary::value_type( filename, ret ));
	}

	return ret;	
}


SrShader* SrResourceManager::GetShader( const char* name )
{
	SrShader* ret = NULL;

	SrResourceLibrary::iterator it = m_shaderLibrary.find(name);

	if (it != m_shaderLibrary.end())
	{
		ret = static_cast<SrShader*>(it->second);
	}

	return ret;
}

SrTexture* SrResourceManager::CreateRenderTexture( const char* name, int width, int height, int bpp )
{
	SrTexture* ret = NULL;
	SrResourceLibrary::iterator it = m_textureLibrary.find(name);

	if (it != m_textureLibrary.end())
	{
		ret = static_cast<SrTexture*>(it->second);
	}
	else
	{
		ret = new SrRenderTexture(name, width, height, bpp);
		m_textureLibrary.insert(SrResourceLibrary::value_type( name, ret ));
	}

	return ret;	
}

SrMaterial* SrResourceManager::CreateManmualMaterial( const char* name )
{
	SrMaterial* ret = NULL;
	return ret;
}

void SrResourceManager::InitDefaultMedia()
{
	m_defaultMediaPack = new SrDefaultMediaPack;
}

void SrResourceManager::LoadMaterialLib( const char* filename )
{
	SrObjLoader loader;

	// load file in memory
	SrMemFile matFile;
	std::string realname(filename);
	getMediaPath(realname);
	matFile.Open( realname.c_str() );
	if (matFile.IsOpen())
	{
		loader.LoadMaterialFromMTL( matFile.Data());
	}
	else
	{
// 		char msg[250];
// 		sprintf_s(msg, "文件[ %s ]未找到！使用默认资源。", realname.c_str());
// 		MessageBox(NULL, msg, "SrResourceManager", MB_OK );
// 
// 		loader.LoadMaterialFromMTL( gEnv->resourceMgr->getDefaultMediaPack()->getDefaultMtl(), *this );
	}
}
// 
// typedef void (*fnLoadShaders)(GlobalEnvironment* pgEnv);
// 
// void SrResourceManager::ReloadShaders()
// {
// 	UnloadSwShaders();
// 	
// 	if ( gEnv->renderer->m_rendererType == eRt_Software )
// 	{
// 
// 		LoadSwShaders();
// 
// 	}
// 	else
// 	{
// 
// 	}
// }

// void SrResourceManager::UnloadSwShaders()
// {
// 	// unload
// 	for (uint32 i=0; i < m_handles.size(); ++i)
// 	{
// 		FreeLibrary( m_handles[i] );
// 	}
// }
// 
// void SrResourceManager::LoadSwShaders()
// {
// 	// 读取外挂命令插件
// 	std::string dir = "\\media\\shader\\";
// 	std::string path = "\\media\\shader\\*.ssl";
// 	getMediaPath(dir);
// 	getMediaPath(path);
// 
// 	WIN32_FIND_DATAA fd;
// 	HANDLE hff = FindFirstFileA(path.c_str(), &fd);
// 	BOOL bIsFind = TRUE;
// 
// 	while(hff && bIsFind)
// 	{
// 		if(fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
// 		{
// 			// do not get into
// 		}
// 		else
// 		{
// 			std::string fullpath = dir + fd.cFileName;
// 
// 			// load dll shaders
// 			HMODULE hDllHandle = 0;
// 			hDllHandle = LoadLibraryA( fullpath.c_str() );
// 			if (hDllHandle)
// 			{
// 
// 				fnLoadShaders fn = (fnLoadShaders)(GetProcAddress( hDllHandle, "LoadShaders" ));
// 
// 				fn( gEnv );
// 				m_handles.push_back(hDllHandle);
// 			}		
// 		}
// 		bIsFind = FindNextFileA(hff, &fd);
// 	}
// }

void SrResourceManager::LoadShaderList()
{

}


void SrResourceManager::AddShader( SrShader* shader )
{
	SrResourceLibrary::iterator it = m_shaderLibrary.find(shader->getName());

	if (it != m_shaderLibrary.end())
	{
	}
	else
	{
		m_shaderLibrary.insert(SrResourceLibrary::value_type( shader->getName(), shader ));
	}

}