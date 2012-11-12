/**
  @file SrRenderContext.h
  
  @author yikaiming

  ������־ history
  ver:1.0
   
 */

#ifndef SrRenderContext_h__
#define SrRenderContext_h__

class SrFragmentBuffer;

struct SrRendContext
{
	SrRendContext(int w, int h, int bpp);

	// ��Ⱦ�������
	void OpenFeature( ERenderFeature feature )
	{
		features |= feature;
	}
	void CloseFeature( ERenderFeature feature )
	{
		features &= ~feature;
	}
	BOOL IsFeatureEnable( ERenderFeature feature )
	{
		return features & feature;
	}
	
	// ��ϸ����
	int width;							///< ��Ⱦ�����
	int height;							///< ��Ⱦ���߶�
	SrViewport viewport;				///< 3ά�ӿ�
	int bpp;							///< ����λ����λbyte
	uint32 features;					///< ��Ⱦ����
	int processorNum;					///< ����������
};
extern SrRendContext* g_context;

#endif // SrRenderContext_h__


