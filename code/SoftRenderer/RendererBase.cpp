/**
  @file RendererBase.cpp
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */
#include "stdafx.h"
#include "RendererBase.h"

IRenderer::IRenderer( ERendererType type ):m_rendererType(type)
{
	m_matrixStack.assign( eMd_Count, float44::CreateIdentity() );
	m_frameCount = 0;

	// create HFONT
	LOGFONT lfont;
	memset   (&lfont,   0,   sizeof   (LOGFONT));   
	lfont.lfHeight=14;
	lfont.lfWeight=800;   
	lfont.lfClipPrecision=CLIP_LH_ANGLES; 
	lfont.lfQuality = NONANTIALIASED_QUALITY; // THIS COULD BOOST
	strcpy_s( lfont.lfFaceName, "consolas" );
	m_bigFont = CreateFontIndirect( &lfont );

	lfont.lfHeight=12;
	lfont.lfWeight=0;  
	m_smallFont = CreateFontIndirect( &lfont );

}

IRenderer::~IRenderer( void )
{
	DeleteObject(m_smallFont);
	DeleteObject(m_bigFont);
}

SrVertexBuffer* IRenderer::AllocateVertexBuffer(uint32 elementSize, uint32 count, bool fastmode)
{
	SrVertexBuffer* vb = new SrVertexBuffer;
	vb->elementSize = elementSize;
	vb->elementCount = count;
	vb->data = (uint8*)(_mm_malloc( elementSize * count, 16 ));

	bool hasEmpty = false;
	for (uint32 i=0; i < m_vertexBuffers.size(); ++i)
	{
		if( m_vertexBuffers[i] == NULL )
		{
			m_vertexBuffers[i] = vb;
			hasEmpty = true;
			break;
		}
	}

	if (!hasEmpty)
	{
		m_vertexBuffers.push_back( vb );
	}	

	return vb;
}

bool IRenderer::DeleteVertexBuffer( SrVertexBuffer* target )
{
	if (target)
	{
		for (uint32 i=0; i < m_vertexBuffers.size(); ++i)
		{
			if( m_vertexBuffers[i] == target )
			{
				_mm_free( m_vertexBuffers[i]->data );
				delete (m_vertexBuffers[i]);
				m_vertexBuffers[i] = NULL;

				// 卸载了就跳出啊！
				return true;
			}
		}
	}


	return false;
}

SrIndexBuffer* IRenderer::AllocateIndexBuffer( uint32 count )
{
	SrIndexBuffer* ib = new SrIndexBuffer;
	ib->data = new uint32[count];
	ib->count = count;

	bool hasEmpty = false;
	for (uint32 i=0; i < m_indexBuffers.size(); ++i)
	{
		if( m_indexBuffers[i] == NULL )
		{
			m_indexBuffers[i] = ib;
			hasEmpty = true;

			// 装入了就跳出啊！
			break;
		}
	}

	if (!hasEmpty)
	{
		m_indexBuffers.push_back( ib );
	}	

	return ib;


}

bool IRenderer::DeleteIndexBuffer( SrIndexBuffer* target )
{
	if (target)
	{
		for (uint32 i=0; i < m_indexBuffers.size(); ++i)
		{
			if( m_indexBuffers[i] == target )
			{
				if (m_indexBuffers[i]->data)
				{
					delete[] m_indexBuffers[i]->data;
					m_indexBuffers[i]->data = NULL;
				}			
				delete m_indexBuffers[i];
				m_indexBuffers[i] = NULL;

				// 卸载了就跳出啊！
				return true;
			}
		}
	}
	return false;
}


void IRenderer::SetMatrix( EMatrixDefine index, const float44& matrix )
{
	if (index < eMd_Count)
	{
		m_matrixStack[index] = matrix;
	}	
}

float44 IRenderer::GetMatrix( EMatrixDefine index )
{
	float44 ret = float44::CreateIdentity();
	if (index < eMd_Count)
	{
		ret = m_matrixStack[index];
	}

	return ret;
}

bool IRenderer::InitRenderer( HWND hWnd, int width, int height, int bpp )
{
	bool ret = false;
	m_hWnd = hWnd;

	ret = InnerInitRenderer(hWnd, width, height, bpp);

	return ret;
}

bool IRenderer::ShutdownRenderer()
{
	// 卸载VB
	for (uint32 i=0; i < m_vertexBuffers.size(); ++i)
	{
		DeleteVertexBuffer( m_vertexBuffers[i] );
	}

	for (uint32 i=0; i < m_indexBuffers.size(); ++i)
	{
		DeleteIndexBuffer( m_indexBuffers[i] );
	}
	return InnerShutdownRenderer();
}

bool IRenderer::DrawScreenText(const char* str, int x,int y, uint32 size, DWORD color)
{
	SrTextLine line;
	line.text = std::string(str);
	line.pos = int2(x,y);
	line.size = size;
	line.color = color;

	m_textLines.push_back( line );

	return true;
}
