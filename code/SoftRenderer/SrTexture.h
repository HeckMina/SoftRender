/**
  @file SrTexture.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrTexture_h__
#define SrTexture_h__
#include "prerequisite.h"
#include "SrResource.h"

enum ESrBitmapType
{
	eBt_file,
	eBt_internal,
};

class SrTexture : public SrResource
{
public:
	SrTexture(const char* name);
	virtual ~SrTexture(void) {}

	const uint8* getBuffer() const {return m_data;}
	int getBPP() const {return m_bpp;}
	int getWidth() const {return m_width;}
	int getHeight() const {return m_height;}  
	int getPitch() const {return m_pitch;}
	ESrBitmapType getTextureType() const {return m_texType;}

	uint32 Get( int2& p ) const
	{
		uint32 ret = 0;
		uint8* data = m_data;

		assert( p.x >=0 && p.x < m_width );
		assert( p.y >=0 && p.y < m_height );

		if ( m_bpp == 3 )
		{
			ret = uint8BGR_2_uint32(data + (m_bpp * p.x + p.y * m_pitch));
			//ret = *(uint32*)(data + (m_bpp * p.x + p.y * m_pitch));
		}
		else
		{
			//ret = uint8BGRA_2_uint32(data + (m_bpp * p.x + p.y * m_pitch));
			ret = *(uint32*)(data + (m_bpp * p.x + p.y * m_pitch));
		}
		
		return ret;
	}
	uint32 Get( const float2& p, ESamplerFilter mode) const;
	mutable void*	m_userData;

protected:
	SrTexture(void);

	int m_width;
	int m_height;
	int m_bpp;
	int m_pitch;

	uint8*	m_data;
	ESrBitmapType m_texType;
	
};

#endif // SrTexture_h__


