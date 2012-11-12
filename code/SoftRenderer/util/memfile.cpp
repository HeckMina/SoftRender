#include "stdafx.h"
#include "memfile.h"

#include "../mmgr/mmgr.h"

SrMemFile::SrMemFile( const SrMemFile& other ) :m_open(false),
	m_data(NULL),
	m_size(0),
	m_binary(false),
	m_unicode(false),
	m_encoding(0)
{
	m_size = other.m_size;
	m_open = other.m_open;
	m_binary = other.m_binary;
	m_unicode = other.m_unicode;
	m_encoding = other.m_encoding;


	if (other.m_data)
	{
		m_data = new char[m_size];
		memcpy( m_data, other.m_data, m_size );
	}
}

SrMemFile& SrMemFile::operator=( const SrMemFile& rhs )
{
	Close();

	m_size = rhs.m_size;
	m_open = rhs.m_open;
	m_binary = rhs.m_binary;
	m_unicode = rhs.m_unicode;
	m_encoding = rhs.m_encoding;

	if (rhs.m_data)
	{
		m_data = new char[m_size];
		memcpy( m_data, rhs.m_data, m_size );
	}

	return *this;
}

void SrMemFile::Open( const char* diskPath )
{
	m_size = 0;
	m_open = false;
	m_unicode = false;

	FILE* pFile = NULL;
	fopen_s( &pFile, diskPath, "rb");

	if (pFile)
	{
		// Get the file size
		fseek(pFile, 0, SEEK_END);
		m_size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		// read the data
		if ( m_size == -1)
		{
			fclose(pFile);
			return;
		}

		if ( m_size > 1024 * 1024 * 1024)
		{
			fclose(pFile);
			return;
		}

		//HGLOBAL hMem = GlobalAlloc( GHND , m_size);
		//char* pTmp = (char*)GlobalLock(hMem);
		char* pTmp = new char[m_size];
		if (!pTmp)
		{
			m_size = 0;
			fclose(pFile);
			return;
		}
		size_t BytesRead = fread(pTmp, 1, m_size, pFile);

		if (BytesRead != m_size)
		{
			delete [] pTmp;
			m_size = 0;
		}
		else
		{
			m_data = pTmp;
			m_open = true;


			// ÅÐ¶Ïunicode
			if ( m_size > 2 )
			{
				if (m_data[0] == 0xEF && m_data[1] == 0xBB)
				{
					m_unicode = true;
					m_encoding = 0;
				}
				else if ( m_data[0] == 0xFE && m_data[1] == 0xFF )
				{
					m_unicode = true;
					m_encoding = 1;
				}
				else if ( m_data[0] == 0xFF && m_data[1] == 0xFE )
				{
					m_unicode = true;
					m_encoding = 2;
				}

				if (!m_unicode)
				{
					const int MAX_UNICODE_TEXT_SIZE = 1001;
					char strUnicodeBuffer[MAX_UNICODE_TEXT_SIZE];
					memset( strUnicodeBuffer , 0 , sizeof(char) * MAX_UNICODE_TEXT_SIZE );
					int nMaxUnicodeSzie = MAX_UNICODE_TEXT_SIZE > m_size ? m_size : MAX_UNICODE_TEXT_SIZE;
					memcpy( strUnicodeBuffer , m_data , nMaxUnicodeSzie );
					strUnicodeBuffer[nMaxUnicodeSzie-1] = '\0';
					int iU = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;  
					m_unicode =  IsTextUnicode( strUnicodeBuffer, nMaxUnicodeSzie, &iU) ? true : false;
				}

			}


			for ( int n = 0; n < m_size; n++ )
			{
				int byte = (int)m_data[n];
				if ((byte <= 31 && byte >= 0) && byte != 9 && byte != 10 && byte != 13)  
				{
					m_binary = true;  
					break;
				}
			}

			if (m_unicode)
			{
				m_binary = false;
			}

		}

		fclose(pFile);				
	}
	else
	{

	}
}
