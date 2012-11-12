#include "StdAfx.h"
#include "SrGpuTimer.h"
#include "d3d9.h"

gkGpuTimer::gkGpuTimer()
{
	m_pEventStart = NULL;
	m_pEventStop = NULL;
	m_pEventFreq = NULL;
	m_fElapsedTime = 0.0f;

	m_skip = true;
}

void gkGpuTimer::init(IDirect3DDevice9* pDevice)
{
	// initialze the querys [2/2/2012 Kaiming]
	pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_pEventStart);
	pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_pEventStop);
	pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &m_pEventFreq);
}

void gkGpuTimer::destroy()
{
	m_pEventStart->Release();
	m_pEventStop->Release();
	m_pEventFreq->Release();
}

void gkGpuTimer::start()
{
// get start time stamp and freq
	if (m_pEventStart)
		m_pEventStart->Issue(D3DISSUE_END);
	if (m_pEventFreq)
		m_pEventFreq->Issue(D3DISSUE_END);
	m_skip = false;
}

void gkGpuTimer::stop()
{
// get end time stamp
	if (m_pEventStop)
		m_pEventStop->Issue(D3DISSUE_END);

	m_skip = false;
}

#define GKGPUTIMER_TRYCOUNT 100000

void gkGpuTimer::update()
{
	UINT64 startTime = 0;
	UINT64 endTime = 0;
	UINT64 freq = 1;

	if (gEnv.renderer->getFrameCount() % 2 == 0)
	{
		if (!m_skip)
		{
			// at the end of a frame, getdata from device
			int trycount = 0;
			bool failed = 0;
			while(trycount < GKGPUTIMER_TRYCOUNT && S_FALSE == m_pEventStart->GetData( (void *)&startTime, sizeof(UINT64), D3DGETDATA_FLUSH ))
			{
				trycount++;
			}

			if (trycount == GKGPUTIMER_TRYCOUNT)
			{
				failed = 1;
			}
			trycount = 0;

			while(trycount < GKGPUTIMER_TRYCOUNT && S_FALSE == m_pEventStop->GetData( (void *)&endTime, sizeof(UINT64), D3DGETDATA_FLUSH ))
			{
				trycount++;
			}

			if (trycount == GKGPUTIMER_TRYCOUNT)
			{
				failed = 1;
			}
			trycount = 0;

			while(trycount < GKGPUTIMER_TRYCOUNT && S_FALSE == m_pEventFreq->GetData( (void *)&freq, sizeof(UINT64), D3DGETDATA_FLUSH ))
			{
				trycount++;
			}

			if (trycount == GKGPUTIMER_TRYCOUNT)
			{
				failed = 1;
			}
			trycount = 0;

			// calculate the elapsedtime in microseconds
			if(!failed)
				m_fElapsedTime = (float)(endTime - startTime) / (float)(freq) * 1000.0f;
		}
		else
		{
			m_fElapsedTime = 0.0f;
		}
	}
	m_skip = true;
}