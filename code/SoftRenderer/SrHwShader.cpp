#include "StdAfx.h"
#include "SrHwShader.h"
#include <d3d9.h>

SrHwShader::SrHwShader( const char* name, SrShader* bindShader ) : SrResource( name, eRT_Shader ),
	m_bindShader(bindShader),
	m_vs(NULL),
	m_ps(NULL)
{
}

SrHwShader::~SrHwShader(void)
{
}

void SrHwShader::Init(IDirect3DDevice9* device)
{
	// create directly
	SrMemFile file;
	std::string path(m_name);
	path = "\\shader\\" + m_name;
	getMediaPath(path);
	file.Open( (path + ".srvs").c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(device->CreateVertexShader( (const DWORD*)(file.Data()), &m_vs )) )
		{
			GtLog("[D3D9 Hw Renderer] Hw VShader [%s] Compile Failed.", m_name.c_str());
		}
	}
	else
	{
		GtLogWarning("[D3D9 Hw Renderer] Hw VShader [%s] Not exist.", m_name.c_str());
		GtLogInfo("[D3D9 Hw Renderer] Compiling Hw Shader...");
		char buffer[1024];
		sprintf(buffer, "fxc.exe /E vs_main /Zpc /T vs_3_0 /Fo shader\\%s.srvs shader\\hwcode\\%s.hlsl",
			m_name.c_str(),
			m_name.c_str());

		if( system( buffer ) == 0 )
		{
			file.Open( (path + ".srvs").c_str() );
			if( file.IsOpen() )
			{
				if ( FAILED(device->CreateVertexShader( (const DWORD*)(file.Data()), &m_vs )) )
				{
					GtLog("[D3D9 Hw Renderer] Hw VShader [%s] Compile Failed.", m_name.c_str());
				}
			}
		}
	}
	file.Close();

	file.Open( (path + ".srps").c_str() );
	if (file.IsOpen())
	{
		if ( FAILED(device->CreatePixelShader( (const DWORD*)(file.Data()), &m_ps )) )
		{
			GtLog("[D3D9 Hw Renderer] Hw PShader [%s] Compile Failed.", m_name.c_str());
		}
	}
	else
	{
		GtLogWarning("[D3D9 Hw Renderer] Hw PShader [%s] Not exist.", m_name.c_str());
		GtLogInfo("[D3D9 Hw Renderer] Compiling Hw Shader...");
		char buffer[1024];
		sprintf(buffer, "fxc.exe /E ps_main /Zpc /T ps_3_0 /Fo shader\\%s.srps shader\\hwcode\\%s.hlsl",
			m_name.c_str(),
			m_name.c_str());

		if( system( buffer ) == 0 )
		{
			file.Open( (path + ".srps").c_str() );
			if( file.IsOpen() )
			{
				if ( FAILED(device->CreatePixelShader( (const DWORD*)(file.Data()), &m_ps )) )
				{
					GtLog("[D3D9 Hw Renderer] Hw PShader [%s] Compile Failed.", m_name.c_str());
				}
			}
		}
	}
	file.Close();

	GtLog("[D3D9 Hw Renderer] Hw Shader [%s] Created.", m_name.c_str());
}

void SrHwShader::Shutdown()
{
	// destroy D3D object
	if (m_vs)
	{
		m_vs->Release();
		GtLog("[D3D9 Hw Renderer] Hw VShader [%s] Destroyed.", m_name.c_str());
		m_vs = NULL;
	}

	if (m_ps)
	{
		m_ps->Release();
		GtLog("[D3D9 Hw Renderer] Hw PShader [%s] Destroyed.", m_name.c_str());
		m_ps = NULL;
	}
}
