#include "StdAfx.h"
#include "SrFragmentBuffer.h"
#include "SrSoftRenderer.h"

SrFragmentBuffer::SrFragmentBuffer( int width, int height, SrSoftRenderer* renderer):m_width(width), m_height(height), m_renderer(renderer)
{
	uint32 size = width * height;
	//fragBuffer = new SrFragmentBuffer[size];
	fBuffer = (SrFragment*)(_mm_malloc( sizeof(SrFragment) * size, 16 ));

	memset(fBuffer, 0, size * sizeof(SrFragment));

	//fragBufferPitch = sizeof(SrFragment) * width;
	m_fBufferIndices = m_renderer->AllocateIndexBuffer(width * height);

	fragBufferSync = new SrFragmentBufferSync[size];

	zBuffer = new float[size];
}


SrFragmentBuffer::~SrFragmentBuffer(void)
{
	_mm_free(fBuffer);
	m_renderer->DeleteIndexBuffer(m_fBufferIndices);
	delete[] fragBufferSync;
	delete[] zBuffer;
}

float3 SrFragmentBuffer::GetNormal( float2& texcoord ) const
{
	// 在这里作warp
	float u = texcoord.x - floor(texcoord.x);
	float v = texcoord.y - floor(texcoord.y);

	// 临近点采样
	u *= (m_width);
	v *= (m_height);

	// get int
	int x = (int)( u );
	int y = (int)( v );
	x = x % m_width;
	y = y % m_height;

	return fBuffer[y * m_width + x].normal_ty.xyz;
}

float3 SrFragmentBuffer::GetWorldPos( float2& texcoord ) const
{
	// 在这里作warp
	float u = texcoord.x - floor(texcoord.x);
	float v = texcoord.y - floor(texcoord.y);

	// 临近点采样
	u *= (m_width);
	v *= (m_height);

	// get int
	int x = (int)( u );
	int y = (int)( v );
	x = x % m_width;
	y = y % m_height;

	return fBuffer[y * m_width + x].worldpos_tx.xyz;
}

void SrFragmentBuffer::Clear()
{
	memset( zBuffer, 0, m_width * m_height * sizeof(float));
}
