// SoftRenderer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SoftRenderApp.h"
#include "SrSponzaApp.h"

#include "mmgr/mmgr.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	// mem leak detecting code...
	// Enable run-time memory check for debug builds.
#if (defined(DEBUG) || defined(_DEBUG))
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	SoftRenderApp app;
	app.RegisterTask(new SrSponzaApp);
	app.Init(hInstance);
	app.Run();
	app.Destroy();

	return 0;
}
