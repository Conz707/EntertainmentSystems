// 10527183 - SOFT351 Coursework 2 "Dino Defense in Space"
//Play in 800x600
//Starting Point - Tutorial 09 'Meshes Using DXUT Helper Classes.cpp' - Nigel Barlow




//--------------------------------------------------------------------------------------
// File: Tutorial 09 - Meshes Using DXUT Helper Classes.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include <xnamath.h>
#include "resource.h"
#include <time.h>
#include "windows.h"






//**************************************************************************//
// Global Variables.  There are many global variables here (we aren't OO	//
// yet.  I doubt  Roy Tucker (Comp Sci students will know him) will			//
// approve pf this either.  Sorry, Roy.										//
//**************************************************************************//
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D   
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
CModelViewerCamera          g_Camera;				// Not used by Nigel.
CDXUTDirectionWidget        g_LightControl;			// Not used by Nigel.


float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;
bool                        g_bShowHelp = false;    // If true, it renders the UI control text
int							g_width  = 800;
int							g_height = 600;

//**************************************************************************//
// Meshes here.																//
//**************************************************************************//
CDXUTSDKMesh                g_MeshDino;			
CDXUTSDKMesh                g_MeshEnemy;					
CDXUTSDKMesh                g_MeshWing1;				
CDXUTSDKMesh                g_MeshWing2;								
CDXUTSDKMesh                g_MeshBullet;				
CDXUTSDKMesh                g_MeshFloor;				
CDXUTSDKMesh                g_MeshSpace;


XMMATRIX					g_MatProjection;

ID3D11InputLayout          *g_pVertexLayout11 = NULL;
ID3D11Buffer               *g_pVertexBuffer   = NULL;
ID3D11Buffer               *g_pIndexBuffer    = NULL;
ID3D11VertexShader         *g_pVertexShader   = NULL;
ID3D11PixelShader          *g_pPixelShader    = NULL;
ID3D11SamplerState         *g_pSamLinear      = NULL;

//**********************************************************************//
// Variables to control the movement of the Dino.						//
// The only one I have coded is rotate about Y, we need an x, y, z		//
// position and maybe rotates about other axes.							//
float g_f_floorRotateY = 0; //used to make look like moving across planet
float g_f_DinoX = 0; //Position of Dino.
float g_f_DinoY = 0;
float g_f_DinoZ = 0;
float g_f_BulletZ = -100;
float g_f_BulletX = -100;
float g_f_EnemyX = 0;
float g_f_EnemyZ = 30;
float g_f_EnemySpeed = 0.03;

int score = 0;


bool		 g_b_LeftArrowDown      = false;	//Status of keyboard.  
bool		 g_b_RightArrowDown     = false;	
bool		g_b_spaceKeyDown = false;
bool		g_b_EnterKeyDown = false;
bool		 g_b_AKeyDown = false;
bool		 g_b_DKeyDown = false;
bool		fireBullet = false;
bool		gameRunning = false;
bool		gameOver = false;



//**************************************************************************//
// This is M$ code, but is usuig old D3DX from DirectX9.  I'm glad to see   //
// that M$ are having issues updating their sample code, same as me - Nigel.//
//**************************************************************************//
CDXUTTextHelper*            g_pTxtHelper = NULL;


//**************************************************************************//
// This is a structure we pass to the vertex shader.  						//
// Note we do it properly here and pass the WVP matrix, rather than world,	//
// view and projection matrices separately.									//
//**************************************************************************//
struct CB_VS_PER_OBJECT
{
	XMMATRIX matWorldViewProj;
    XMMATRIX matWorld;				// needed to transform the normals.

};






//**************************************************************************//
// These are structures we pass to the pixel shader.  						//
// Note we do it properly here and pass the WVP matrix, rather than world,	//
// view and projection matrices separately.									//
//																			//
// These structures must be identical to those defined in the shader that	//
// you use.  So much for encapsulation; Roy	Tucker (Comp Sci students will  //
// know him) will not approve.												//
//**************************************************************************//
struct CB_PS_PER_OBJECT
{
    XMFLOAT4 m_vObjectColor;
};
UINT                        g_iCBPSPerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    XMVECTOR m_vLightDirAmbient;	// Vector pointing at the light
};


struct MexhVertexStructure
{
	XMFLOAT4 pos;
	XMFLOAT3 normal;
	XMFLOAT2 TextureUV;
};

UINT                        g_iCBPSPerFrameBind = 1;



//**************************************************************************//
// Now a global instance of each constant buffer.							//
//**************************************************************************//
ID3D11Buffer               *g_pcbVSPerObject = NULL;
ID3D11Buffer               *g_pcbPSPerObject = NULL;
ID3D11Buffer               *g_pcbPSPerFrame  = NULL;



//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4





//**************************************************************************//
// If you are not used to "C" you will find that functions (or methods in	//
// "C++" must have templates defined in advance.  It is usual to define the //
// prototypes in a header file, but we'll put them here for now to keep		//
// things simple.															//
//**************************************************************************//
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

extern bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                             bool bWindowed, void* pUserContext );
extern HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice,
                                            const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
extern HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                           void* pUserContext );
extern void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
                                        void* pUserContext );
extern void CALLBACK OnD3D9LostDevice( void* pUserContext );
extern void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();
void charStrToWideChar(WCHAR *dest, char *source);
void RenderMesh (ID3D11DeviceContext* pd3dImmediateContext, CDXUTSDKMesh *toRender);





//**************************************************************************//
// A Windows program always kicks off in WinMain.							//
// Initializes everything and goes into a message processing				//
// loop.																	//
//																			//
// This version uses DXUT, and is much more complicated than previous		//
// versions you have seen.  This allows you to sync the frame rate to your  //
// monitor's vertical sync event.											//
//**************************************************************************//
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    
	//**************************************************************************//
	// Set DXUT callbacks.														//
    //**************************************************************************//
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );


    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );


    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow(L"Tutorial 09 - Meshes Using DXUT Helper Classes" );
    DXUTCreateDevice (D3D_FEATURE_LEVEL_9_2, true, 800, 600 );
    //DXUTCreateDevice(true, 640, 480);
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{

    // Initialize dialogs
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 23 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += 26, 170, 23, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 23, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // Uncomment this to get debug information from D3D11
    //pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
    }
    return true;
}






//**************************************************************************//
// Handle updates to the scene.  This is called regardless of which D3D		//
// API is used (we are only using Dx11).									//
//**************************************************************************//
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	if (gameRunning == true)
	{
		g_f_floorRotateY += XMConvertToRadians(0.002);
		g_f_EnemyZ -= g_f_EnemySpeed;

		if (fireBullet == true)
		{
			g_f_BulletZ += 0.04;
		}

		if (g_b_LeftArrowDown && g_f_DinoX >= -13.28 || g_b_AKeyDown && g_f_DinoX >= -13.28)
		{
			g_f_DinoX -= 0.04;
		}
		if (g_b_RightArrowDown && g_f_DinoX <= 13.28 || g_b_DKeyDown && g_f_DinoX <= 13.28)
		{
			g_f_DinoX += 0.04;
		}
		if (g_b_spaceKeyDown) PlaySound(L"Media\\lasersounds.wav", NULL, SND_ASYNC | SND_NOSTOP);	//Eagle sound on space key down

		////check for collision - reset bullet on collision		
		//end game if enemy hits player Z
		if (g_f_EnemyZ < g_f_DinoZ - 20)
		{
			g_f_EnemyZ = 30;
			g_f_BulletX = -100;
			g_f_BulletZ = -100;
			gameRunning = false;
			gameOver = true;
			g_f_EnemySpeed = 0.03;
		

		}

		if ((max(g_f_BulletX / 3, g_f_EnemyX)) - (min(g_f_BulletX/3, g_f_EnemyX)) < 2)
		{
			if (g_f_BulletZ > g_f_EnemyZ)
			{
				g_f_EnemySpeed += 0.003;
				g_f_EnemyX = (((float(rand()) / float(RAND_MAX)) * (13.28 + 13.28)) - 13.28) / 3;	//enemy in random X position on screen after death
				g_f_EnemyZ = 30;
				g_f_BulletX = -100;
				g_f_BulletZ = -100;
				fireBullet = false;
				score += 100;
				PlaySound(L"Media\\explode.wav", NULL, SND_ASYNC | SND_NOSTOP);
			}
		}
	}
}




//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    UINT nBackBufferHeight = ( DXUTIsAppRenderingWithD3D9() ) ? DXUTGetD3D9BackBufferSurfaceDesc()->Height :
            DXUTGetDXGIBackBufferSurfaceDesc()->Height;

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    // Draw help
    if( g_bShowHelp )
    {
        g_pTxtHelper->SetInsertionPos( 2, nBackBufferHeight - 20 * 10 );
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );


        g_pTxtHelper->SetInsertionPos( 20, nBackBufferHeight - 20 * 7 );
        g_pTxtHelper->DrawTextLine( L"Movement Direction: Left / Right arrows\n"
									L"Shoot: SPACE\n"
									);

        g_pTxtHelper->SetInsertionPos( 550, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Hide help: F1\n"
                                    L"Quit: ESC\n" );
    }
    else
    {
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Press F1 for help" );


		WCHAR text[50];
		WCHAR s[50];
		if (gameRunning == true) {
			
			swprintf(s, L"Score: %i", score);
			g_pTxtHelper->SetInsertionPos(325, 100);
			g_pTxtHelper->DrawTextLine(s);
		} else if (gameOver== true){
			swprintf(text, L"Game Over! \n"
				L"Press Return to play again!");
			swprintf(s, L" \n"				L"Your score was: %i", score);


			g_pTxtHelper->SetInsertionPos(325, 200);
			g_pTxtHelper->DrawTextLine(text);
			g_pTxtHelper->DrawTextLine(s);
		}
		else {

			g_pTxtHelper->SetInsertionPos(250, 250);
			g_pTxtHelper->DrawTextLine( L"Movement - Left / Right Arrows or A / D\n"
				L"Shoot / - Space\n"
				L"Start Game - Enter / Return\n"
				L"Survive as long as possible");
		}
		
		
		
    }

    g_pTxtHelper->End();
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
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;


    return 0;
}







//**************************************************************************//
// Handle key presses.														//
//**************************************************************************//
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp;
				break;
			case VK_SPACE:
				if (gameRunning)
					fireBullet = true;
					g_f_BulletX = g_f_DinoX;
					g_f_BulletZ = g_f_DinoZ - 5;
					break;
				
			case VK_RETURN:
				if (!gameRunning)
					gameRunning = true;
					break;

      }

    }

	
	//**************************************************************//
	// Nigel code to rotate the Dino.								//
	//**************************************************************//
	switch( nChar )
	{		       
		case VK_LEFT:  g_b_LeftArrowDown  = bKeyDown; break;
		case VK_RIGHT: g_b_RightArrowDown = bKeyDown; break;
		case VK_SPACE  :  g_b_spaceKeyDown = bKeyDown; break;
		case VK_RETURN:  g_b_EnterKeyDown = bKeyDown; break;
		case 0x41: g_b_AKeyDown = bKeyDown; break;
		case 0x44: g_b_DKeyDown = bKeyDown; break;
       }

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
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;
    }

}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}



//**************************************************************************//
// Compile the shader file.  These files aren't pre-compiled (well, not		//
// here, and are compiled on he fly).										//
//**************************************************************************//
HRESULT CompileShaderFromFile( WCHAR* szFileName,		// File Name
							  LPCSTR szEntryPoint,		// Namee of shader
							  LPCSTR szShaderModel,		// Shader model
							  ID3DBlob** ppBlobOut )	// Blob returned
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( str, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
		WCHAR errorCharsW[200];
        if( pErrorBlob != NULL )
		{
			charStrToWideChar(errorCharsW, (char *)pErrorBlob->GetBufferPointer());
            MessageBox( 0, errorCharsW, L"Error", 0 );
		}
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );






	//**********************************************************************//
    // Compile the pixel and vertex shaders.  If your computer doesn't		//
	// support shader model 5, try shader model 4.  There is nothing we are //
	// using here that needs shader model 5.								//
	//**********************************************************************..
    ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Tutorial 09 - Meshes Using DXUT Helper Classes_VS.hlsl", "VS_DXUTSDKMesh", "vs_4_0", &pVertexShaderBuffer ) );

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Tutorial 09 - Meshes Using DXUT Helper Classes_PS.hlsl", "PS_DXUTSDKMesh", "ps_4_0", &pPixelShaderBuffer ) );

    
	
	//**********************************************************************//
    // Create the pixel and vertex shaders.									//
	//**********************************************************************//
	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "VS_DXUTSDKMesh" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "PS_DXUTSDKMesh" );

	
	
	//**********************************************************************//
    // Define the input layout.  I won't go too much into this except that  //
	// the vertex defined here MUST be consistent with the vertex shader	//
	// input you use in your shader file and the constand buffer structure  //
	// at the top of this module.											//
	//																		//
	// Normal vectors are used by lighting.									//
	//**********************************************************************//
     const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );



    //**************************************************************************//
	// Initialize the projection matrix.  Generally you will only want to create//
	// this matrix once and then forget it.										//
	//**************************************************************************//
	g_MatProjection = XMMatrixPerspectiveFovLH( XM_PIDIV2,				// Field of view (pi / 2 radians, or 90 degrees
											 g_width / (FLOAT) g_height, // Aspect ratio.
											 0.01f,						// Near clipping plane.
											 10000.0f );					// Far clipping plane.


	//**************************************************************************//
    // Load the mesh.															//
	//**************************************************************************//
    V_RETURN( g_MeshDino.Create( pd3dDevice, L"Media\\parasaur\\parasaur.sdkmesh", true ) );
	V_RETURN(g_MeshEnemy.Create(pd3dDevice, L"Media\\AlienDino\\parasaur.sdkmesh", true));
	V_RETURN(g_MeshBullet.Create(pd3dDevice, L"Media\\dog\\dog.sdkmesh", true));
	V_RETURN(g_MeshWing1.Create(pd3dDevice, L"Media\\wing\\wing.sdkmesh", true));
	V_RETURN(g_MeshWing2.Create(pd3dDevice, L"Media\\wing\\wing.sdkmesh", true));

	V_RETURN(g_MeshFloor.Create(pd3dDevice, L"Media\\floor\\seafloor.sdkmesh", true));
	V_RETURN(g_MeshSpace.Create(pd3dDevice, L"Media\\space\\seafloor.sdkmesh", true));

	// Create a sampler state
    D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &SamDesc, &g_pSamLinear ) );
    DXUT_SetDebugName( g_pSamLinear, "Primary" );

    
	//**************************************************************************//
	// Create the 3 constant bufers, using the same buffer descriptor to create //
	// all three.																//
	//**************************************************************************//
    D3D11_BUFFER_DESC Desc;
 	ZeroMemory( &Desc, sizeof(Desc) );
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    Desc.MiscFlags = 0;
	
    Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbVSPerObject ) );
    DXUT_SetDebugName( g_pcbVSPerObject, "CB_VS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( CB_PS_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerObject ) );
    DXUT_SetDebugName( g_pcbPSPerObject, "CB_PS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( CB_PS_PER_FRAME );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerFrame ) );
    DXUT_SetDebugName( g_pcbPSPerFrame, "CB_PS_PER_FRAME" );


    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	g_width  = pBackBufferSurfaceDesc->Width;
	g_height = pBackBufferSurfaceDesc->Height;

	g_HUD.SetLocation( g_width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
	g_SampleUI.SetLocation( g_width - 170, g_height - 300 );
    g_SampleUI.SetSize( 170, 300 );

    return S_OK;
}




//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    // Clear the render target and depth stencil
    float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );




	
	//******************************************************************//
	// Create the world matrix for the Dino: just a rotate around	    //
	// the Y axis of 45 degrees.  DirectX does all angles in radians,	//
	// hence the conversion.  And a translate.							//
	//******************************************************************//
	//Meshes Matrices
	
	//Dino
	XMMATRIX matDinoTranslate	 = XMMatrixTranslation(g_f_DinoX, 5, -10);
	XMMATRIX matDinoScale		 = XMMatrixScaling(1, 1, 1);

	XMMATRIX matDinoWorld		 = matDinoTranslate * matDinoScale;

	//Dino Movement

	XMVECTOR Eye = XMVectorSet(0.0f, 20.0f, -17.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 10.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);



	XMMATRIX matView;
	matView = XMMatrixLookAtLH(Eye,	// The eye, or viewer's position.
		At,		// The look at point.
		Up);	// Which way is up.


	//Wing 1

	XMMATRIX matWing1Translate = XMMatrixTranslation(0.2, 1.8, 0);
	XMMATRIX matWing1Scale = XMMatrixScaling(1, 1, 1);


	XMMATRIX matWing1World =  matWing1Translate * matWing1Scale * matDinoWorld ;

	XMMATRIX matWing1WVP = matWing1World * matView * g_MatProjection;

	//Wing 2
	XMMATRIX matWing2Translate = XMMatrixTranslation(0.2, -1.8, 0);
	XMMATRIX matWing2Scale = XMMatrixScaling(-1, -1, 1);;

	XMMATRIX matWing2World =  matWing2Translate * matWing2Scale * matDinoWorld;
	XMMATRIX matWing2WVP = matWing2World * matView * g_MatProjection;

	//Bullet
	XMMATRIX matBulletTranslate = XMMatrixTranslation(g_f_BulletX, g_f_DinoY+1.5, g_f_BulletZ);
	XMMATRIX matBulletScale = XMMatrixScaling(1, 3, 3);

	XMMATRIX matBulletWorld = matBulletTranslate * matBulletScale;
	XMMATRIX matBulletWVP = matBulletWorld * matView * g_MatProjection;



	//Floor
	XMMATRIX matFloorTranslate = XMMatrixTranslation(0, 9, -10);
	XMMATRIX matFloorScale = XMMatrixScaling(0.5, 0.5, 0.5);
	XMMATRIX matFloorRotateY = XMMatrixRotationY(g_f_floorRotateY);
	
	XMMATRIX matFloorWorld = matFloorRotateY * matFloorTranslate * matFloorScale;
	XMMATRIX matFloorWVP = matFloorWorld * matView * g_MatProjection;


	//Space
	XMMATRIX matSpaceTranslate = XMMatrixTranslation(3.5, -70, 60);
	XMMATRIX matSpaceScale = XMMatrixScaling(2.5, 2.5, 2.5);
	XMMATRIX matSpaceRotateX = XMMatrixRotationX(-90);

	XMMATRIX matSpaceWorld = matSpaceRotateX * matSpaceTranslate * matSpaceScale;
	XMMATRIX matSpaceWVP = matSpaceWorld * matView * g_MatProjection;

	//Enemy
	XMMATRIX matEnemyTranslate = XMMatrixTranslation(g_f_EnemyX, 5, g_f_EnemyZ);
	XMMATRIX matEnemyScale = XMMatrixScaling(3, 1, 3);
	XMMATRIX matEnemyRotateY = XMMatrixRotationY(XMConvertToRadians(180));
	XMMATRIX matEnemyRotateX = XMMatrixRotationX(XMConvertToRadians(-25));

	XMMATRIX matEnemyWorld = matEnemyRotateX * matEnemyRotateY * matEnemyTranslate * matEnemyScale;
	XMMATRIX matEnemyWVP = matEnemyWorld * matView * g_MatProjection;



	XMMATRIX matWorldViewProjection;
	matWorldViewProjection = matDinoWorld * matView * g_MatProjection;



	//******************************************************************//    
	// Update shader variables.  We must update these for every mesh	//
	// that we draw (well, actually we need only update the position	//
	// for each mesh, think hard about this - Nigel						//
	//																	//
    // We pass the parameters to it in a constant buffer.  The buffer	//
	// we define in this module MUST match the constant buffer in the	//
	// shader.															//
	//																	//
	// It would seem that the constant buffer we pass to the shader must//
	// be global, well defined on the heap anyway.  Not a local variable//
	// it would seem.													//
	//******************************************************************//
	CB_VS_PER_OBJECT CBMatrices;
	CBMatrices.matWorld = XMMatrixTranspose(matDinoWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matWorldViewProjection);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	//******************************************************************//
	// Lighting.  Ambient light and a light direction, above, to the	//
	// left and two paces back, I think.  Then normalise the light		//
	// vector.  It is kind-a-silly doing this every frame unless the	//
	// light moves.														//
	//******************************************************************//
    float    fAmbient                = 0.1f;
	XMVECTOR vectorLightDirection    = XMVectorSet(-1, 1, -2, 0);  // 4th value unused.
	vectorLightDirection = XMVector3Normalize(vectorLightDirection);
										


	CB_PS_PER_FRAME CBPerFrame;
	CBPerFrame.m_vLightDirAmbient = vectorLightDirection;
	pd3dImmediateContext->UpdateSubresource( g_pcbPSPerFrame, 0, NULL, &CBPerFrame, 0, 0 );
	pd3dImmediateContext->PSSetConstantBuffers( 1, 1, &g_pcbPSPerFrame );


	CB_PS_PER_OBJECT CBPerObject;
	CBPerObject.m_vObjectColor = XMFLOAT4(1, 1, 1, 1);
	pd3dImmediateContext->UpdateSubresource( g_pcbPSPerObject, 0, NULL, &CBPerObject, 0, 0 );
	pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &g_pcbPSPerObject );


    pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

	//**************************************************************************//
	// Render the meshes.															//
	//**************************************************************************//
	//Dino Render
	RenderMesh (pd3dImmediateContext, &g_MeshDino);


	//Wing 1 Render
	CBMatrices.matWorld = XMMatrixTranspose(matWing1World);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matWing1WVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshWing1);

	//Wing 2 Render
	CBMatrices.matWorld = XMMatrixTranspose(matWing2World);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matWing2WVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshWing2);

	//Bullet Render
	CBMatrices.matWorld = XMMatrixTranspose(matBulletWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matBulletWVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshBullet);

	//Enemy Render

	CBMatrices.matWorld = XMMatrixTranspose(matEnemyWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matEnemyWVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshEnemy);

	//Render the Shadow Meshes



	//Floor Render
	CBMatrices.matWorld = XMMatrixTranspose(matFloorWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matFloorWVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshFloor);

	//Space Render
	CBMatrices.matWorld = XMMatrixTranspose(matSpaceWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matSpaceWVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderMesh(pd3dImmediateContext, &g_MeshSpace);

	//Viewer Matrix
	CBMatrices.matWorld = XMMatrixTranspose(matFloorWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matFloorWVP);
	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);
	
	//**************************************************************************//
	// Render what is rather grandly called the head up display.				//
	//**************************************************************************//
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();
}





//**************************************************************************//
// Render a CDXUTSDKMesh, using the Device Context specified.				//
//**************************************************************************//
void RenderMesh (ID3D11DeviceContext *pContext, 
				 CDXUTSDKMesh         *toRender)
{
	//Get the mesh
    //IA setup
    pContext->IASetInputLayout( g_pVertexLayout11 );
    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = toRender->GetVB11( 0, 0 );
    Strides[0] = ( UINT )toRender->GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    pContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    pContext->IASetIndexBuffer( toRender->GetIB11( 0 ), toRender->GetIBFormat11( 0 ), 0 );

    // Set the shaders
    pContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pContext->PSSetShader( g_pPixelShader,  NULL, 0 );
    
	for( UINT subset = 0; subset < toRender->GetNumSubsets( 0 ); ++subset )
    {
		//Render
		SDKMESH_SUBSET* pSubset = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;
        
		// Get the subset
        pSubset = toRender->GetSubset( 0, subset );

        PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
        pContext->IASetPrimitiveTopology( PrimType );

		//**************************************************************************//
		// At the moment we load a texture into video memory every frame, which is	//
		// HORRIBLE, we need to create more Texture2D variables.					//
		//**************************************************************************//
        ID3D11ShaderResourceView* pDiffuseRV = toRender->GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
        pContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

        pContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
    }

}



//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

    g_MeshDino.Destroy();
	g_MeshEnemy.Destroy();
	g_MeshWing1.Destroy();
	g_MeshWing2.Destroy();
	g_MeshFloor.Destroy();
	g_MeshBullet.Destroy();
	g_MeshSpace.Destroy();

    SAFE_RELEASE( g_pVertexLayout11 );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pSamLinear );

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}




//**************************************************************************//
// Convert an old chracter (char *) string to a WCHAR * string.  There must //
// be something built into Visual Studio to do this for me, but I can't		//
// find it - Nigel.															//
//**************************************************************************//
void charStrToWideChar(WCHAR *dest, char *source)
{
	int length = strlen(source);
	for (int i = 0; i <= length; i++)
		dest[i] = (WCHAR) source[i];
}
