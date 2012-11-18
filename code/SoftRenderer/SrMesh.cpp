#include "StdAfx.h"
#include "SrMesh.h"
#include "SrObjLoader.h"
#include "SrMaterial.h"
#include "SrResourceManager.h"
#include "SrDefaultMedia.h"

#include "mmgr/mmgr.h"


SrMesh::SrMesh(const char* filename):SrResource(filename, eRt_Mesh)
	,m_subsetCount(0)
{
	m_primitives.clear();

	ParseObjFile();
}

SrMesh::~SrMesh(void)
{
	Destroy();
}

void SrMesh::Draw()
{
	for (uint32 i=0; i < m_primitives.size(); ++i)
	{
		SrMaterial* mat = m_primitives[i].material;
		if( mat )
		{
			
			// set shader to renderer
			gEnv->renderer->SetShader(mat->m_shader);

			// apply texture to renderer
			mat->ApplyTextures();

			// set shaderConstants
			mat->ApplyShaderConstants();

			gEnv->renderer->DrawPrimitive( &(m_primitives[i]) );
		}		
	}
}

void SrMesh::setMaterial( SrMaterial& mat, uint32 index )
{
	if (m_primitives.size() > index)
	{
		m_primitives[index].material = &mat;
	}
}

void SrMesh::ParseObjFile()
{
	SrObjLoader loader;

	// load file in memory
	SrMemFile meshFile;
	std::string realname(m_name);
	getMediaPath(realname);
	meshFile.Open( realname.c_str() );
	if (meshFile.IsOpen())
	{
		loader.LoadGeometryFromOBJ( meshFile.Data() , m_primitives );
	}
	else
	{
		char msg[250];
		sprintf_s(msg, "文件[ %s ]未找到！使用默认资源。", realname.c_str());
		MessageBox(NULL, msg, "SrResourceManager", MB_OK );


		loader.LoadGeometryFromOBJ( gEnv->resourceMgr->getDefaultMediaPack()->getDefaultMesh(), m_primitives );
	}
	

	// should have realtime - tangent calc
}

void SrMesh::Destroy()
{
	for (uint32 i=0; i < m_primitives.size(); ++i)
	{
		gEnv->renderer->DeleteVertexBuffer( m_primitives[i].vb );
		gEnv->renderer->DeleteIndexBuffer( m_primitives[i].ib );
	}

	m_primitives.clear();
}
