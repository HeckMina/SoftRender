/**
  @file SoftRenderApp.h
  
  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SoftRenderApp_h__
#define SoftRenderApp_h__

#include "SrAppFramework.h"
#include "InputManager.h"

SR_ALIGN class SoftRenderApp : public IInputEventListener
{
public:
	SoftRenderApp(void);
	virtual ~SoftRenderApp(void);
	BOOL Init( HINSTANCE hInstance );
	void Destroy();
	void Run();

	bool Update();
	
	void RegisterTask(SrAppFramework* task);
	void UnRegisterTasks();

private:
	class IRenderer* m_pRenderer;
	IRenderer* m_pSwRenderer;
	IRenderer* m_pHwRenderer;

	HINSTANCE m_hInst;								// current instance
	HWND m_hWnd;
	SrApps m_tasks;
	
	void LoadShaderList();

	virtual bool OnInputEvent( const SInputEvent &event );

};

#endif // SoftRenderApp_h__