/**
  @file SrGpuTimer.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrGpuTimer_h__
#define SrGpuTimer_h__

struct IDirect3DQuery9;
struct IDirect3DDevice9;

class gkGpuTimer
{
public:
	gkGpuTimer();
	virtual ~gkGpuTimer() {}

	float getTime() {return m_fElapsedTime;}

	// init D3DQUERY [2/2/2012 Kaiming]
	void init(IDirect3DDevice9* pDevice);
	void destroy();

	// mark the timestamp
	void start();
	void stop();

	// get timestep when finish this frame
	void update();

private:
	IDirect3DQuery9* m_pEventStart;
	IDirect3DQuery9* m_pEventStop;
	IDirect3DQuery9* m_pEventFreq;

	float m_fElapsedTime;

	bool m_skip;


};

#endif // SrGpuTimer_h__


