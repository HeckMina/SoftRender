#include "StdAfx.h"
#include "SoftRenderApp.h"
#include "resource.h"
#include "SrSoftRenderer.h"
#include <MMSystem.h>
#include "SrMesh.h"
#include "InputManager.h"
#include "SrProfiler.h"
#include "SrHwD3D9Renderer.h"

#include "mmgr/mmgr.h"

GlobalEnvironment* gEnv = NULL;
// Forward declarations of functions included in this code module:

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
// 		switch (wmId)
// 		{
// 		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
//		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


SoftRenderApp::SoftRenderApp(void):
	m_pRenderer(NULL),
	m_pHwRenderer(NULL),
	m_pSwRenderer(NULL)
{
}


SoftRenderApp::~SoftRenderApp(void)
{
}

BOOL SoftRenderApp::Init( HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOFTRENDERER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_SOFTRENDERER);
	wcex.lpszClassName	= "SoftRenderer Window Class";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);


	m_hInst = hInstance; // Store instance handle in our global variable

	const int createWidth = 640;
	const int createHeight = 360;

	m_hWnd = CreateWindow("SoftRenderer Window Class", "SoftRenderer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, createWidth, createHeight, NULL, NULL, hInstance, NULL);

	RECT realRect;
	GetClientRect(m_hWnd, &realRect);

	int width = realRect.right - realRect.left;
	int height = realRect.bottom - realRect.top;
	width = createWidth * 2 - width;
	height = createHeight * 2 - height;

	MoveWindow(m_hWnd, GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2, width, height, FALSE);

	if (!m_hWnd)
	{
		return FALSE;
	}

	gEnv = new GlobalEnvironment;

	// 创建资源管理器
	gEnv->resourceMgr = new SrResourceManager;
	//LoadInternalShaders();
	gEnv->resourceMgr->InitDefaultMedia();

	// 创建Render上下文
	g_context = new SrRendContext(createWidth, createHeight, 32);

	m_pHwRenderer = new SrHwD3D9Renderer;
	
 	m_pSwRenderer = new SrSoftRenderer;
 	m_pRenderer = m_pSwRenderer;
	gEnv->renderer = m_pRenderer;

	m_pSwRenderer->InitRenderer(m_hWnd, createWidth, createHeight, 32);
	m_pHwRenderer->InitRenderer(m_hWnd, createWidth, createHeight, 32);
	
	gEnv->resourceMgr->ReloadShaders();

	gEnv->timer = new SrTimer;
	gEnv->timer->Init();

	gEnv->inputSys = new SrInputManager;
	gEnv->inputSys->Init(m_hWnd);
	gEnv->inputSys->AddListener(this);

	gEnv->profiler = new SrProfiler;


	// 显示窗口
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);

	// 切换焦点
	SetFocus(m_hWnd);
	SetForegroundWindow(m_hWnd);

	SrApps::iterator it = m_tasks.begin();
	for (; it != m_tasks.end(); ++it)
	{
		(*it)->OnInit();
	}

	// hexin sizeof

	char buffer[255];

#define OUTPUT_SIZE( x ) \
	sprintf(buffer, #x" size: %d\n", sizeof(x)); \
	OutputDebugString(buffer);

	OUTPUT_SIZE( float2 )
	OUTPUT_SIZE( float3 )
	OUTPUT_SIZE( float4 )
	OUTPUT_SIZE( float33 )
	OUTPUT_SIZE( float44 )
	OUTPUT_SIZE( Quat )


	return TRUE;
}

static Quat g_rot = Quat::CreateIdentity();

bool SoftRenderApp::Update()
{

	gEnv->profiler->setBegin(ePe_FrameTime);

	gEnv->timer->Update();

	gEnv->inputSys->Update();

	if (!m_pRenderer)
	{
		return false;
	}	
	m_pRenderer->BeginFrame();
	
	m_pRenderer->HwClear();

	SrApps::iterator it = m_tasks.begin();
	for (; it != m_tasks.end(); ++it)
	{
		(*it)->OnUpdate();
	}

	gEnv->profiler->Update();

	//g_context->GetSBuffer();
	m_pRenderer->EndFrame();

	gEnv->profiler->setEnd(ePe_FrameTime);

	return true;
}

void SoftRenderApp::Destroy()
{
	SrApps::iterator it = m_tasks.begin();
	for (; it != m_tasks.end(); ++it)
	{
		(*it)->OnDestroy();

		// should delete task by me!
		delete (*it);
	}

	delete g_context;
	delete gEnv->resourceMgr;

	if (m_pHwRenderer)
	{
		m_pHwRenderer->ShutdownRenderer();
		delete m_pHwRenderer;
		m_pHwRenderer = NULL;
	}

	if (m_pSwRenderer)
	{
		m_pSwRenderer->ShutdownRenderer();
		delete m_pSwRenderer;
		m_pSwRenderer = NULL;
	}

	if (gEnv->timer)
	{
		delete gEnv->timer;
	}

	if (gEnv->inputSys)
	{
		gEnv->inputSys->Destroy();
		delete gEnv->inputSys;
	}

	if (gEnv->profiler)
	{
		delete gEnv->profiler;
	}

	delete gEnv;
}

void SoftRenderApp::Run()
{
	for(;;)
	{
		MSG msg;

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message != WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				
				break;
			}
		}
		else
		{
		
			if (!Update())
			{
				// need to clean the message loop (WM_QUIT might cause problems in the case of a restart)
				// another message loop might have WM_QUIT already so we cannot rely only on this 
				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}

		//Update();
	}
}

void SoftRenderApp::RegisterTask( SrAppFramework* task )
{
	SrApps::iterator it = m_tasks.begin();
	for (; it != m_tasks.end(); ++it)
	{
		if ( *it == task)
		{
			return;
		}
	}

	m_tasks.push_back(task);
}

void SoftRenderApp::UnRegisterTasks()
{
	SrApps::iterator it = m_tasks.begin();
	for (; it != m_tasks.end(); ++it)
	{
		delete (*it);
	}
}

bool SoftRenderApp::OnInputEvent( const SInputEvent &event )
{
	switch(event.keyId)
	{
		case eKI_Tilde:
			{
				if (event.state == eIS_Pressed)
				{
					if (m_pRenderer == m_pHwRenderer)
					{
						m_pRenderer = m_pSwRenderer;
					}
					else
					{
						m_pRenderer = m_pHwRenderer;
					}

					gEnv->renderer = m_pRenderer;
				}
			}
			break;

	}
	return false;
}

