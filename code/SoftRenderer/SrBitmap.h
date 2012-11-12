/**
  @file SrBitmap.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrBitmap_h__
#define SrBitmap_h__
#include "prerequisite.h"
#include "SrTexture.h"


namespace ATL
{
	class CImage;
}

class SrBitmap : public SrTexture
{
public:
	// file bitmap
	SrBitmap(const char* name);
	virtual ~SrBitmap(void);

public:

private:
	SrBitmap(void);
	ATL::CImage* m_image;
};

#endif // SrBitmap_h__