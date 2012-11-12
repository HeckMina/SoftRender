/**
  @file SrFragmentBuffer.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrFragmentBuffer_h__
#define SrFragmentBuffer_h__
#include "prerequisite.h"

struct  SrRendContext;
class SrSoftRenderer;

class SrFragmentBuffer
{
	friend SrRendContext;
public:
	SrFragmentBuffer(int width, int height, SrSoftRenderer* renderer);
	~SrFragmentBuffer(void);

	LPCRITICAL_SECTION GetSyncMark(uint32 address)
	{
		return &(fragBufferSync[address].cs);
	}

	inline float3 GetNormal(float2& texcoord) const;
	float3 GetWorldPos(float2& texcoord) const;

	SrIndexBuffer* GetPixelIndicesBuffer() {return m_fBufferIndices;}

	void Clear();

public:
	SrFragment*				fBuffer;		///< fragBuffer
	float*					zBuffer;

private:
	SrIndexBuffer*			m_fBufferIndices;	///< fragBufferIndices索引
	
	SrFragmentBufferSync*	fragBufferSync;

	uint32					m_width;
	uint32					m_height;

	SrSoftRenderer*			m_renderer;
};
extern SrFragmentBuffer* fBuffer;


#endif // SrFragmentBuffer_h__


