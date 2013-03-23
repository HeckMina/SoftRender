//--------------------------------------------------------------------------------------
// File: SimpleSample.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 

#define SQRT_PARTICLE_COUNT 64

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*            g_pTxtHelper = NULL;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls

// Direct3D 9 resources
ID3DXFont*                  g_pFont9 = NULL;
ID3DXSprite*                g_pSprite9 = NULL;
ID3DXEffect*                g_pEffect9 = NULL;
D3DXHANDLE                  g_hmWorldViewProjection;
D3DXHANDLE                  g_hmWorld;
D3DXHANDLE                  g_hfTime;

bool						g_paused = false;

D3DVERTEXELEMENT9 g_gpuParticleVD[] =
{
	{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0},
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
	{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 1},
	D3DDECL_END()
};

D3DVERTEXELEMENT9 g_drawQuadVD[] =
{
	{ 0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITIONT, 0},
	{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};
IDirect3DVertexDeclaration9*	g_drawQuadVertexDecl;

struct sParticleVert
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex;
	D3DXVECTOR2 texVTF;
};

struct PPVERT
{
	D3DXVECTOR4 post;
	D3DXVECTOR2 texcoord0;       // Texcoord for post-process source
};

//////////////////////////////////////////////////////////////////////////
// PARTICLE 更新纹理

// 1. POS/MASS MAP xyz|位置 w|质量
// 2. VEL/LIVE xyz|速度 w|生存状态

IDirect3DIndexBuffer9*			g_gpuParticleIB;
IDirect3DVertexBuffer9*			g_gpuParticleVB;
IDirect3DVertexDeclaration9*	g_gpuVertexDecl;
IDirect3DTexture9*				g_gpuTexture0;
IDirect3DTexture9*				g_gpuTexture1;
IDirect3DTexture9*				g_gpuTexture2;

IDirect3DTexture9*				g_velocityMap0;
IDirect3DTexture9*				g_velocityMap1;

IDirect3DTexture9*				g_updateTexture;
IDirect3DTexture9*				g_pTexture;

IDirect3DTexture9*				g_trialTexture;

ID3DXMesh*						g_planet;

struct GPUUpdateContent
{
	IDirect3DTexture9* g_prevPOSRT;
	IDirect3DTexture9* g_currPOSRT;

	IDirect3DTexture9* g_prevVELRT;
	IDirect3DTexture9* g_currVELRT;

	void Swap()
	{
		IDirect3DTexture9* tmp = g_prevPOSRT;
		g_prevPOSRT = g_currPOSRT;
		g_currPOSRT = tmp;

		tmp = g_prevVELRT;
		g_prevVELRT = g_currVELRT;
		g_currVELRT = tmp;
	}

	void SwapVelocity()
	{
		IDirect3DTexture9* tmp = g_prevVELRT;
		g_prevVELRT = g_currVELRT;
		g_currVELRT = tmp;
	}

	void PushVelRT(IDirect3DDevice9* pd3dDevice, int index = 0)
	{
		IDirect3DSurface9* rt;
		g_currVELRT->GetSurfaceLevel(0, &rt);
		pd3dDevice->SetRenderTarget(index, rt);
		rt->Release();
	}

	void PushPosRT(IDirect3DDevice9* pd3dDevice, int index = 0)
	{
		IDirect3DSurface9* rt;
		g_currPOSRT->GetSurfaceLevel(0, &rt);
		pd3dDevice->SetRenderTarget(index, rt);
		rt->Release();
	}
};

GPUUpdateContent				g_content;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

#define IDC_PAUSE_PARTICLE		4


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext );
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D9LostDevice( void* pUserContext );
void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"gpu_particle_system" );
    DXUTCreateDevice( true, 1280, 800 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
	g_HUD.AddButton( IDC_PAUSE_PARTICLE, L"Pause Particle (F5)", 35, iY += 24, 125, 22, VK_F5 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}

void DrawQuad( IDirect3DDevice9* pd3dDevice, int nTexWidth, int nTexHeight, int nTexLeft =0, int nTexTop = 0 )
{
	PPVERT quad[4];

	quad[0].post = D3DXVECTOR4(  -0.5f + nTexLeft,						-0.5f + nTexTop,						1.0f, 1.0f );
	quad[1].post = D3DXVECTOR4(  -0.5f + nTexLeft,						- 0.5f + nTexTop + nTexHeight,		1.0f, 1.0f );	
	quad[2].post = D3DXVECTOR4(  - 0.5f + nTexLeft + nTexWidth,		-0.5f + nTexTop,						1.0f, 1.0f );	
	quad[3].post = D3DXVECTOR4(  - 0.5f + nTexLeft + nTexWidth,		- 0.5f + nTexTop + nTexHeight,		1.0f, 1.0f );

	// screenTC
	quad[0].texcoord0 = D3DXVECTOR2( 0.0f,	0.0f );
	quad[1].texcoord0 = D3DXVECTOR2( 0.0f,	1.0f );
	quad[2].texcoord0 = D3DXVECTOR2( 1.0f,	0.0f );
	quad[3].texcoord0 = D3DXVECTOR2( 1.0f,	1.0f );

	pd3dDevice->SetVertexDeclaration( g_drawQuadVertexDecl );
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(PPVERT) );
}
//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.5f, 0.5f, 1.0f, 1.0f ) );
	
	WCHAR buffer[255];
	swprintf(buffer, L"particle count: %d", SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT );
	g_pTxtHelper->DrawTextLine( buffer );

    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                      D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
    {
        IDirect3D9* pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        // then switch to SWVP.
        if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

		//pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        // Debugging vertex shaders requires either REF or software vertex processing 
        // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
            ( DXUT_D3D10_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE ) )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont9 ) );

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#ifdef DEBUG_VS
    dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
    dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif
#ifdef D3DXFX_LARGEADDRESS_HANDLE
    dwShaderFlags |= D3DXFX_LARGEADDRESSAWARE;
#endif

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"gpu_particle_glalaxy.fx" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags,
                                        NULL, &g_pEffect9, NULL ) );

	if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"Particle.dds", &g_pTexture ) ) )
	{
		// If texture is not in current folder, try parent folder
		if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"..\\Particle.dds", &g_pTexture ) ) )
		{
			MessageBox( NULL, L"Could not find banana.bmp", L"Textures.exe", MB_OK );
			return E_FAIL;
		}
	}

    g_hmWorldViewProjection = g_pEffect9->GetParameterByName( NULL, "g_mWorldViewProjection" );
    g_hmWorld = g_pEffect9->GetParameterByName( NULL, "g_mWorld" );
    g_hfTime = g_pEffect9->GetParameterByName( NULL, "g_fTime" );

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice,
                                    const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont9 ) V_RETURN( g_pFont9->OnResetDevice() );
    if( g_pEffect9 ) V_RETURN( g_pEffect9->OnResetDevice() );

    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite9 ) );
    g_pTxtHelper = new CDXUTTextHelper( g_pFont9, g_pSprite9, NULL, NULL, 15 );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 350 );
    g_SampleUI.SetSize( 170, 300 );
	
	pd3dDevice->CreateTexture( 1280, 800, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &g_trialTexture, NULL );
	

	pd3dDevice->CreateIndexBuffer( SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT * 2 * 3 * sizeof(DWORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &g_gpuParticleIB, NULL );
	pd3dDevice->CreateVertexBuffer( SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT * 4 * sizeof(sParticleVert), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &g_gpuParticleVB, NULL );

	// posmap
	pd3dDevice->CreateTexture( SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &g_gpuTexture0, NULL );
	pd3dDevice->CreateTexture( SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &g_gpuTexture1, NULL );

	// velmap
	pd3dDevice->CreateTexture( SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &g_velocityMap0, NULL );
	pd3dDevice->CreateTexture( SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &g_velocityMap1, NULL );


	pd3dDevice->CreateTexture( SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT, 1, NULL, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &g_updateTexture, NULL );



	
	pd3dDevice->CreateVertexDeclaration( g_gpuParticleVD, &g_gpuVertexDecl );
	pd3dDevice->CreateVertexDeclaration( g_drawQuadVD, &g_drawQuadVertexDecl );


	sParticleVert* startVertex;
	if (SUCCEEDED( g_gpuParticleVB->Lock( 0, SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT * 4 * sizeof(sParticleVert), (void**)&startVertex, NULL) ))
	{
		// set index
		for (DWORD i=0; i < SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT; ++i)
		{
			startVertex[i * 4 + 0].tex = D3DXVECTOR2(0,0);
			startVertex[i * 4 + 1].tex = D3DXVECTOR2(0,1);
			startVertex[i * 4 + 2].tex = D3DXVECTOR2(1,0);
			startVertex[i * 4 + 3].tex = D3DXVECTOR2(1,1);

			startVertex[i * 4 + 0].texVTF = D3DXVECTOR2( (i % SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT, (i / SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT );
			startVertex[i * 4 + 1].texVTF = D3DXVECTOR2( (i % SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT, (i / SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT );
			startVertex[i * 4 + 2].texVTF = D3DXVECTOR2( (i % SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT, (i / SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT );
			startVertex[i * 4 + 3].texVTF = D3DXVECTOR2( (i % SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT, (i / SQRT_PARTICLE_COUNT) / (float)SQRT_PARTICLE_COUNT );
		}

		g_gpuParticleVB->Unlock();
	}


	DWORD* startIndex;
	if( SUCCEEDED( g_gpuParticleIB->Lock( 0, SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT * 2 * 3 * sizeof(DWORD), (void**)&startIndex, NULL ) ) )
	{
		DWORD index = 0;
		for (DWORD i=0; i < SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT; ++i)
		{
			startIndex[i * 6] = index;
			startIndex[i * 6 + 1] = index + 1;
			startIndex[i * 6 + 2] = index + 2;
			startIndex[i * 6 + 3] = index + 2;
			startIndex[i * 6 + 4] = index + 1;
			startIndex[i * 6 + 5] = index + 3;

			index += 4;
		}

		g_gpuParticleIB->Unlock();
	}


	// fill the texture with random value
	D3DLOCKED_RECT rect;
	if( SUCCEEDED( g_updateTexture->LockRect(0, &rect, NULL, NULL) ))
	{
		D3DXVECTOR4* startVertex = (D3DXVECTOR4*)rect.pBits;
		for (DWORD i=0; i < SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT; ++i)
		{
// 			float ran0 = rand() % 10000 / 10000.f - 0.5f;
// 			float ran1 = rand() % 10000 / 10000.f - 0.5f;
// 			float ran2 = rand() % 10000 / 10000.f - 0.5f;
// 			startVertex[i] = D3DXVECTOR4(ran0 * 10, ran1 * 10, ran2 * 10, 1);
 			float ran2 = rand() % 10000 / 10000.f;
			startVertex[i] = D3DXVECTOR4(-10000, -10000, -10000, ran2 > 0.9999 ? 2.0 : 1.0 );
		}


		g_updateTexture->UnlockRect(0);
	}
	pd3dDevice->UpdateTexture( g_updateTexture, g_gpuTexture0 );

	float initSpeed = 0.8f;
	if( SUCCEEDED( g_updateTexture->LockRect(0, &rect, NULL, NULL) ))
	{
		D3DXVECTOR4* startVertex = (D3DXVECTOR4*)rect.pBits;
		for (DWORD i=0; i < SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT; ++i)
		{
			float ran0 = rand() % 10000 / 10000.f - 0.5f;
			float ran1 = rand() % 10000 / 10000.f - 0.5f;
			float ran2 = rand() % 10000 / 10000.f - 0.5f;
			startVertex[i] = D3DXVECTOR4(0, 0, 0, 0);
		}


		g_updateTexture->UnlockRect(0);
	}
	pd3dDevice->UpdateTexture( g_updateTexture, g_velocityMap0 );


	g_content.g_prevPOSRT = g_gpuTexture0;
	g_content.g_currPOSRT = g_gpuTexture1;

	g_content.g_prevVELRT = g_velocityMap0;
	g_content.g_currVELRT = g_velocityMap1;


	D3DXCreateSphere( pd3dDevice, 0.2, 20, 20, &g_planet, NULL );


    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

	if ( g_paused )
	{
		fElapsedTime = 0;
	}

	static int s_emitted = 0;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		static float fTime = 0;
		fTime += fElapsedTime;

		// BACKUP RT
		IDirect3DSurface9* backbuffer;
		pd3dDevice->GetRenderTarget(0, &backbuffer );
		
		//float fTime = 0.5f;
		D3DXVECTOR4 pos[3];
		pos[0] = D3DXVECTOR4(sinf(fTime + 0.5) * 10,cosf(fTime + 0.5) * 8,cosf(fTime + 0.5) * 10,0);
		pos[1] = D3DXVECTOR4(sinf(fTime) + 5.5, cosf(fTime), cosf(fTime + 0.5), 0);
		pos[2] = D3DXVECTOR4(sinf(fTime - 0.5) * 5,cosf(fTime - 0.5) * 2,cosf(fTime + 0.5) * 5,0);

		// RT UPDATE VELOCITY

		{
			pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

			D3DXHANDLE tech;



			// EVERYFRAME
			//g_pEffect9->SetInt( "g_emitCount", 0);
			
			// EXPIRE ROUTINE
			if (true)
			{
				g_content.PushVelRT(pd3dDevice);
				g_content.PushPosRT(pd3dDevice, 1);

				g_pEffect9->SetTechnique( "Expire" );
	
				UINT pass;
				if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
				{
					g_pEffect9->BeginPass( 0 );

					pd3dDevice->SetTexture(0, g_content.g_prevVELRT);
					pd3dDevice->SetTexture(1, g_content.g_prevPOSRT);

					DrawQuad( pd3dDevice, SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT );

					g_pEffect9->EndPass();


					g_pEffect9->End();
				}

				g_content.Swap();
				pd3dDevice->SetRenderTarget(1, NULL);
			}



			// IF NEED EMIT
			static float s_emitTimer = 0;
			s_emitTimer += fElapsedTime;
			if (s_emitTimer > 0.01)
			{
				g_content.PushVelRT(pd3dDevice);
				g_content.PushPosRT(pd3dDevice, 1);


				s_emitTimer = 0;
				// EMIT
				g_pEffect9->SetTechnique( "Emit" );
				g_pEffect9->SetInt( "g_emitCount", 1);
				
				float initSpeed = 5.0f;
				float ran0 = rand() % 10000 / 10000.f - 0.5f;
				float ran1 = rand() % 10000 / 10000.f * 0.5 + 0.5;// - 0.5f;
				float ran2 = rand() % 10000 / 10000.f - 0.5f;
				g_pEffect9->SetVectorArray( "g_emitParam", &(D3DXVECTOR4(ran0,ran1 * initSpeed,ran2,1)), 1);
				g_pEffect9->SetVectorArray( "g_emitPos", &(D3DXVECTOR4(0,0,0,1)), 1);


				
				s_emitted++;
				s_emitted = s_emitted % (SQRT_PARTICLE_COUNT * SQRT_PARTICLE_COUNT);
				int x = s_emitted % (SQRT_PARTICLE_COUNT );
				int y = s_emitted / (SQRT_PARTICLE_COUNT );
				
				//g_pEffect9->SetVectorArray("g_emitIndex", &(D3DXVECTOR4( (float)x / (float)(SQRT_PARTICLE_COUNT - 1), (float)y / (float)(SQRT_PARTICLE_COUNT - 1),0,1)), 1);
				g_pEffect9->SetVectorArray("g_emitIndex", &(D3DXVECTOR4( x, y, 0,1)), 1);
				//g_pEffect9->CommitChanges();
				g_pEffect9->CommitChanges();
				UINT pass;
				if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
				{
					g_pEffect9->BeginPass( 0 );

					pd3dDevice->SetTexture(0, g_content.g_prevVELRT);
					pd3dDevice->SetTexture(1, g_content.g_prevPOSRT);

					DrawQuad( pd3dDevice, SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT );

					g_pEffect9->EndPass();


					g_pEffect9->End();
				}

				g_content.Swap();
				pd3dDevice->SetRenderTarget(1, NULL);
			}


			g_content.PushVelRT(pd3dDevice);
			// UPDATE
			g_pEffect9->SetTechnique( "Eular_Attraction" );

			D3DXVECTOR4 time(fElapsedTime, fElapsedTime, fElapsedTime, fElapsedTime);
			D3DXVECTOR4 force(0, sinf(fTime) * 9.8f,0,0);

			g_pEffect9->SetValue( "g_timeVar", &time, sizeof(D3DXVECTOR4) );
			g_pEffect9->SetValue( "g_staticForce", &force, sizeof(D3DXVECTOR4) );


			g_pEffect9->SetValue( "g_attract0", &(pos[0]), sizeof(D3DXVECTOR4) );
			g_pEffect9->SetValue( "g_attract1", &(pos[1]), sizeof(D3DXVECTOR4) );
			g_pEffect9->SetValue( "g_attract2", &(pos[2]), sizeof(D3DXVECTOR4) );
			
			UINT pass;
			if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
			{
				g_pEffect9->BeginPass( 0 );
				
				pd3dDevice->SetTexture(0, g_content.g_prevVELRT);
				pd3dDevice->SetTexture(1, g_content.g_prevPOSRT);

				DrawQuad( pd3dDevice, SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT );

				g_pEffect9->EndPass();


				g_pEffect9->End();
			}
		}

		// RT UPDATE POSITION
		g_content.PushPosRT(pd3dDevice);
		{
			pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			D3DXHANDLE tech = g_pEffect9->GetTechniqueByName( "Eular_Update" );
			g_pEffect9->SetTechnique( tech );

			D3DXVECTOR4 time(fElapsedTime, fElapsedTime, fElapsedTime, fElapsedTime);

			D3DXVECTOR4 force(0, sinf(fTime) * 9.8f,0,0);

			g_pEffect9->SetValue( "g_timeVar", &time, sizeof(D3DXVECTOR4) );

			UINT pass;
			if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
			{
				g_pEffect9->BeginPass( 0 );
				
				pd3dDevice->SetTexture(0, g_content.g_currVELRT);
				pd3dDevice->SetTexture(1, g_content.g_prevPOSRT);

				DrawQuad( pd3dDevice, SQRT_PARTICLE_COUNT, SQRT_PARTICLE_COUNT );

				g_pEffect9->EndPass();


				g_pEffect9->End();
			}
		}





		// ROLL RTS
		g_content.Swap();

		pd3dDevice->SetRenderTarget(0, backbuffer);
		backbuffer->Release();


		pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		D3DXHANDLE tech = g_pEffect9->GetTechniqueByName( "RenderScene" );
		g_pEffect9->SetTechnique( tech );
		UINT pass;
		if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
		{
			g_pEffect9->BeginPass( 0 );


			for (UINT i=0; i < 3; ++i)
			{
				// draw planet
				D3DXMATRIX world;
				D3DXMatrixIdentity(&world);
				D3DXMatrixTranslation(&world, pos[i].x, pos[i].y, pos[i].z );

				mWorld = world;
				mProj = *g_Camera.GetProjMatrix();
				mView = *g_Camera.GetViewMatrix();

				mWorldViewProjection = mWorld * mView * mProj;

				// Update the effect's variables.  Instead of using strings, it would 
				// be more efficient to cache a handle to the parameter by calling 
				// ID3DXEffect::GetParameterByName
				V( g_pEffect9->SetMatrix( g_hmWorldViewProjection, &mWorldViewProjection ) );
				V( g_pEffect9->SetMatrix( g_hmWorld, &mWorld ) );
				V( g_pEffect9->SetFloat( g_hfTime, ( float )fTime ) );
				g_pEffect9->CommitChanges();

				g_planet->DrawSubset(0);
			}


			g_pEffect9->EndPass();


			g_pEffect9->End();
		}
		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);



        // Get the projection & view matrix from the camera class
        mWorld = *g_Camera.GetWorldMatrix();
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mWorldViewProjection = mWorld * mView * mProj;

        // Update the effect's variables.  Instead of using strings, it would 
        // be more efficient to cache a handle to the parameter by calling 
        // ID3DXEffect::GetParameterByName
        V( g_pEffect9->SetMatrix( g_hmWorldViewProjection, &mWorldViewProjection ) );
        V( g_pEffect9->SetMatrix( g_hmWorld, &mWorld ) );
        V( g_pEffect9->SetFloat( g_hfTime, ( float )fTime ) );

		

		D3DXMATRIX mViewT;
		D3DXMatrixTranspose( &mViewT, &mView );
		D3DXVECTOR4 right = D3DXVECTOR4(mViewT._11, mViewT._12, mViewT._13, 1);
		D3DXVECTOR4 up = D3DXVECTOR4(mViewT._21, mViewT._22, mViewT._23, 1);

		D3DXHANDLE bbright = g_pEffect9->GetParameterByName( NULL, "g_bbRight" );
		D3DXHANDLE bbup = g_pEffect9->GetParameterByName( NULL, "g_bbUp" );
		
		g_pEffect9->SetVector( bbright, &right );
		g_pEffect9->SetVector( bbup, &up );
		V( g_pEffect9->SetFloat( g_hfTime, ( float )fTime ) );
		

		pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		tech = g_pEffect9->GetTechniqueByName( "RenderParticle" );
		g_pEffect9->SetTechnique( tech );
		pass;
		if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
		{
			g_pEffect9->BeginPass( 0 );

			pd3dDevice->SetVertexDeclaration( g_gpuVertexDecl );
			pd3dDevice->SetTexture( D3DVERTEXTEXTURESAMPLER0, g_content.g_prevPOSRT );
			pd3dDevice->SetTexture(1, g_pTexture);
			pd3dDevice->SetStreamSource( 0, g_gpuParticleVB, 0, sizeof(sParticleVert) );
			pd3dDevice->SetIndices( g_gpuParticleIB );

			pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, s_emitted * 4, 0, s_emitted * 2); 

			g_pEffect9->EndPass();


			g_pEffect9->End();
		}

		static bool first = true;
		if (first)
		{
			first = false;

			IDirect3DSurface9* surf;
			g_trialTexture->GetSurfaceLevel(0, &surf );
			pd3dDevice->StretchRect( backbuffer, NULL, surf, NULL, D3DTEXF_POINT );
			surf->Release();
		}


		if (!g_paused)
		{
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);

			tech = g_pEffect9->GetTechniqueByName( "Trail_Merge" );
			g_pEffect9->SetTechnique( tech );

			if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
			{
				g_pEffect9->BeginPass( 0 );

				pd3dDevice->SetTexture(0, g_trialTexture);

				DrawQuad( pd3dDevice, 1280, 800 );

				g_pEffect9->EndPass();


				g_pEffect9->End();
			}
		}

		
		IDirect3DSurface9* surf;
		g_trialTexture->GetSurfaceLevel(0, &surf );
		pd3dDevice->StretchRect( backbuffer, NULL, surf, NULL, D3DTEXF_POINT );
		surf->Release();

		// DRAW PARTICLE LIFE
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			tech = g_pEffect9->GetTechniqueByName( "ShowParticleLife" );
			g_pEffect9->SetTechnique( tech );

			if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
			{
				g_pEffect9->BeginPass( 0 );

				pd3dDevice->SetTexture(0, g_content.g_currVELRT);

				DrawQuad( pd3dDevice, 256, 256, 0, 800 - 256 );

				g_pEffect9->EndPass();


				g_pEffect9->End();
			}
		}

		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			tech = g_pEffect9->GetTechniqueByName( "ShowParticleLife" );
			g_pEffect9->SetTechnique( tech );

			if( SUCCEEDED( g_pEffect9->Begin( &pass, NULL ) ) )
			{
				g_pEffect9->BeginPass( 0 );

				pd3dDevice->SetTexture(0, g_content.g_currPOSRT);

				DrawQuad( pd3dDevice, 256, 256, 256, 800 - 256 );

				g_pEffect9->EndPass();


				g_pEffect9->End();
			}
		}


        DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" ); // These events are to help PIX identify what the code is doing
        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );
        DXUT_EndPerfEvent();

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
		case IDC_PAUSE_PARTICLE:
			g_paused = !g_paused; break;
    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
	g_gpuParticleIB->Release();
	g_gpuParticleVB->Release();
	g_gpuTexture0->Release();
	g_gpuTexture1->Release();
	g_velocityMap0->Release();
	g_velocityMap1->Release();

	g_trialTexture->Release();

	g_updateTexture->Release();
	g_gpuVertexDecl->Release();
	g_drawQuadVertexDecl->Release();

	g_planet->Release();

    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont9 ) g_pFont9->OnLostDevice();
    if( g_pEffect9 ) g_pEffect9->OnLostDevice();
    SAFE_RELEASE( g_pSprite9 );
    SAFE_DELETE( g_pTxtHelper );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pEffect9 );
    SAFE_RELEASE( g_pFont9 );
	SAFE_RELEASE(g_pTexture);
}


