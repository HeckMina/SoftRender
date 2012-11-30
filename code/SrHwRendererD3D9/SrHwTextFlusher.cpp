#include "StdAfx.h"
#include "SrHwTextFlusher.h"

SrHwTextFlusher::SrHwTextFlusher()
{
}

SrHwTextFlusher::~SrHwTextFlusher()
{
}

void SrHwTextFlusher::Init( IDirect3DDevice9* device )
{
	D3DXCreateFont( device, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		"Consolas", &m_pFont9 );

	D3DXCreateFont( device, 14, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		"Consolas", &m_pFont9Big );

	D3DXCreateSprite( device, &m_pSprite9 );
}

void SrHwTextFlusher::Destroy()
{
	m_pFont9->Release();
	m_pFont9Big->Release();
	m_pSprite9->Release();
}

HRESULT SrHwTextFlusher::DrawFormattedTextLine( RECT& rc, DWORD dwFlags, DWORD color, const char* strMsg, bool big, ... )
{
	char strBuffer[512];

	va_list args;
	va_start( args, strMsg );
	vsprintf_s( strBuffer, 512, strMsg, args );
	strBuffer[511] = L'\0';
	va_end( args );

	return DrawTextLine( rc, dwFlags, color, strBuffer, big );
}


HRESULT SrHwTextFlusher::DrawTextLine( RECT& rc, DWORD dwFlags, DWORD color, const char* strMsg, bool big)
{
	HRESULT hr;
	if( m_pFont9Big && big )
		hr = m_pFont9Big->DrawText( m_pSprite9, strMsg, -1, &rc, DT_NOCLIP, color | 0xff000000 );
	else if ( m_pFont9)
		hr = m_pFont9->DrawText( m_pSprite9, strMsg, -1, &rc, DT_NOCLIP, color | 0xff000000 );
	if( FAILED( hr ) )
	{

	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
void SrHwTextFlusher::Begin()
{
	if( m_pSprite9 )
		m_pSprite9->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
}
void SrHwTextFlusher::End()
{
	if( m_pSprite9 )
		m_pSprite9->End();
}


