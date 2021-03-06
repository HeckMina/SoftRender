/**
  @file SoftRenderApp.h
  
  @author yikaiming

  ������־ history
  ver:1.0
   
 */

#ifndef SoftRenderApp_h__
#define SoftRenderApp_h__

#include "SrAppFramework.h"
#include "InputManager.h"

typedef std::vector<IRenderer*> SrRendererList;

class SoftRenderApp : public IInputEventListener
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

	bool InitRenderers();
	void ShutdownRenderers();

private:
	HINSTANCE m_hInst;								// current instance
	HWND m_hWnd;
	SrApps m_tasks;
	SrRendererList m_renderers;
	SrHandleList m_rendHandles;
	uint32 m_currRendererIndex;

	void LoadShaderList();

	virtual bool OnInputEvent( const SInputEvent &event );

};

#endif // SoftRenderApp_h__