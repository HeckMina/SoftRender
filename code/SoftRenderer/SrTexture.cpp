#include "StdAfx.h"
#include "SrTexture.h"
#include "SrProfiler.h"

#include "mmgr/mmgr.h"

SrTexture::SrTexture( const char* name ) : SrResource(name, eRT_Texture), m_userData(NULL)
{
	GtLog("[ResourceManager] Texture[%s] Created.", m_name.c_str());
}

uint32 SrTexture::Get( const float2& p, ESamplerFilter mode /*= eSF_Linear*/ ) const
{
	uint32 final = 0x00000000;

	// 在这里作warp
	float u = p.x - floor(p.x);
	float v = p.y - floor(p.y);

	switch(mode)
	{
	case eSF_Nearest:
		{
			// 临近点采样
			u *= (m_width);
			v *= (m_height);

			// get int
			int x = (int)( u );
			int y = (int)( v );

			x = x % m_width;
			y = y % m_height;

			assert( x >= 0 && x < m_width );
			assert( y >= 0 && y < m_height );

			final = Get(int2(x,y));
		}

		break;
	case eSF_Linear:
		// 双线性内插
	default:
		{
			// 取得纹理空间的地址
			u *= (m_width - 1);
			v *= (m_height - 1);

			// 取整
			int x = (int)( u );
			int y = (int)( v );

			// 计算子像素偏移
			float du = u - x;
			float dv = v - y;

			// 取得地址位置，确保在纹理采样范围内
			int left = x % m_width;
			int right = (x + 1) % m_width;
			int up = y % m_height;
			int down = (y + 1) % m_height;

			assert( left >= 0 && left < m_width );
			assert( right >= 0 && right < m_width );
			assert( up >= 0 && up < m_height );
			assert( down >= 0 && down < m_height );

			// 采样4个颜色
			uint32 lt = Get( int2(left,up) );
			uint32 t = Get( int2(right,up) );
			uint32 l = Get( int2(left,down) );
			uint32 rb =  Get( int2(right,down) );

			// 使用四个颜色和子像素偏移来调和最终颜色
			final = SrColorMulFloat(lt,((1.f-du)*(1.f-dv))) + SrColorMulFloat(t , (du*(1.f - dv))) + SrColorMulFloat(l , ((1.f-du)*dv)) + SrColorMulFloat(rb , (du*dv));
		}
	}
	return final;
}
