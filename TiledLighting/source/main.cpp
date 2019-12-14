#include <memory>

#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Core\\DXUTmisc.h"
#include "..\\..\\DXUT\\Optional\\DXUTgui.h"
#include "..\\..\\DXUT\\Optional\\DXUTCamera.h"
#include "..\\..\\DXUT\\Optional\\DXUTSettingsDlg.h"
#include "..\\..\\DXUT\\Optional\\SDKmisc.h"
#include "..\\..\\DXUT\\Optional\\SDKmesh.h"

#include "TiledRenderer.h"

#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 

using namespace DirectX;

static const int TEXT_LINE_HEIGHT = 15;
float g_maxSceneDistance = 500.0f;
CFirstPersonCamera g_camera;
CDXUTDialogResourceManager g_dialogResourceManager;
CD3DSettingsDlg g_SettingsDlg;
std::shared_ptr<CDXUTTextHelper> g_textHelper;
std::shared_ptr<TiledRenderer> g_renderer;

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);

void InitApp();
void RenderText();
void UpdateUI();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// Set DXUT callbacks
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);

	InitApp();
	DXUTInit(true, true, NULL); 
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"TiledRendering");

	DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1280, 800);
	//DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1920, 1080);

	SetFocus(DXUTGetHWND());

	DXUTMainLoop(); // Enter into the DXUT render loop

	return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Create the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	WCHAR szTemp[256];
	D3DCOLOR DlgColor = 0x88888888;
	g_SettingsDlg.Init(&g_dialogResourceManager);

	UpdateUI();
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_textHelper->Begin();
	g_textHelper->SetInsertionPos(5, 5);
	g_textHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
	g_textHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	g_textHelper->DrawTextLine(DXUTGetDeviceStats());




	g_textHelper->End();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									  DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									 void* pUserContext)
{
	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	V_RETURN(g_SettingsDlg.OnD3D11CreateDevice(pd3dDevice));
	g_textHelper = std::shared_ptr<CDXUTTextHelper>(
		new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, TEXT_LINE_HEIGHT));

	g_renderer = std::shared_ptr<TiledRenderer>(new TiledRenderer);
	if (g_renderer)
	{
		ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

		g_renderer->Create(pd3dDevice,
						   pd3dImmediateContext,
						   pBackBufferSurfaceDesc->Width,
						   pBackBufferSurfaceDesc->Height,
						   pBackBufferSurfaceDesc->SampleDesc.Count);

		const XMVECTOR SceneMin = DirectX::XMLoadFloat3(&g_renderer->GetSceneBoundMin());
		const XMVECTOR SceneMax = DirectX::XMLoadFloat3(&g_renderer->GetSceneBoundMax());

		XMVECTOR SceneCenter = 0.5f * (SceneMax + SceneMin);
		XMVECTOR SceneExtents = 0.5f * (SceneMax - SceneMin);
		XMVECTOR BoundaryMin = SceneCenter - 2.0f*SceneExtents;
		XMVECTOR BoundaryMax = SceneCenter + 2.0f*SceneExtents;
		XMVECTOR BoundaryDiff = BoundaryMax - BoundaryMin;
		g_maxSceneDistance = XMVectorGetX(XMVector3Length(BoundaryDiff));
		XMVECTOR vEye = SceneCenter - XMVectorSet(0.45f*XMVectorGetX(SceneExtents), 0.35f*XMVectorGetY(SceneExtents), 0.0f, 0.0f);
		XMVECTOR vAt = SceneCenter - XMVectorSet(0.0f, 0.35f*XMVectorGetY(SceneExtents), 0.0f, 0.0f);
		g_camera.SetRotateButtons(true, false, false);
		g_camera.SetEnablePositionMovement(true);
		g_camera.SetViewParams(vEye, vAt);
		g_camera.SetScalers(0.005f, 0.1f*g_maxSceneDistance);
		XMFLOAT3 vBoundaryMin, vBoundaryMax;
		XMStoreFloat3(&vBoundaryMin, BoundaryMin);
		XMStoreFloat3(&vBoundaryMax, BoundaryMax);
		g_camera.SetClipToBoundary(true, &vBoundaryMin, &vBoundaryMax);
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(g_SettingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	if (g_renderer)
	{
		g_renderer->Resize(pBackBufferSurfaceDesc->Width,
						   pBackBufferSurfaceDesc->Height,
						   pBackBufferSurfaceDesc->SampleDesc.Count);
	}

	float aspectRatio = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);
	g_camera.SetProjParams(XM_PI / 4.0f, aspectRatio, g_maxSceneDistance, 1.0f);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
								 float fElapsedTime, void* pUserContext)
{
	static float time = 0.0f;
	static int frame = 0;

	time += fElapsedTime;
	frame++;

	if (time > 1.f)
	{
		float fps = (float)(frame) / time;
		std::wstring text = std::to_wstring(fps);
		SetWindowText(DXUTGetHWND(), text.c_str());

		frame = 0;
		time = 0.0f;
	}

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	if (g_renderer)
		g_renderer->Render(g_camera);

	RenderText();
}


//--------------------------------------------------------------------------------------
// Destroy D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Destroy D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	if (g_renderer)
		g_renderer.reset();

	g_dialogResourceManager.OnD3D11DestroyDevice();
	g_SettingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
}

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	static bool isFirstTime = true;
	if (isFirstTime)
	{
		isFirstTime = false;

		if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			DXUTDisplaySwitchingToREFWarning();
		}

		pDeviceSettings->d3d11.SyncInterval = 0;
	}

	if (pDeviceSettings->d3d11.sd.SampleDesc.Count > 1)
	{
		pDeviceSettings->d3d11.sd.SampleDesc.Count = 1;
	}
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 0;

	pDeviceSettings->d3d11.AutoCreateDepthStencil = false;

	return true;
}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	// Update the camera's position based on user input 
	if (g_renderer)
		g_renderer->Update(fElapsedTime);

	g_camera.FrameMove(fElapsedTime);
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
						 void* pUserContext)
{
	switch (uMsg)
	{
	case WM_GETMINMAXINFO:
		// override DXUT_MIN_WINDOW_SIZE_X and DXUT_MIN_WINDOW_SIZE_Y
		// to prevent windows that are too small
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 512;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 512;
		*pbNoFurtherProcessing = true;
		break;
	}
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	//Pass messages to settings dialog if its active
	if (g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (bKeyDown)
	{
		switch (nChar)
		{
		case VK_F1:
			//g_bRenderHUD = !g_bRenderHUD;
			break;
		}
	}
}

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{

}

void UpdateUI()
{
	//bool bShadowMode = (g_LightingMode == LIGHTING_SHADOWS);

	//int maxPointLights = bShadowMode ? MAX_NUM_SHADOWCASTING_POINTS : MAX_NUM_LIGHTS;
	//g_iNumActivePointLights = bShadowMode ? MAX_NUM_SHADOWCASTING_POINTS : MAX_NUM_LIGHTS;
	//g_HUD.m_GUI.GetSlider(IDC_SLIDER_NUM_POINT_LIGHTS)->SetRange(0, maxPointLights);
	//g_HUD.m_GUI.GetSlider(IDC_SLIDER_NUM_POINT_LIGHTS)->SetValue(g_iNumActivePointLights);

	//int maxSpotLights = bShadowMode ? MAX_NUM_SHADOWCASTING_SPOTS : MAX_NUM_LIGHTS;
	//g_iNumActiveSpotLights = bShadowMode ? 0 : 0;
	//g_HUD.m_GUI.GetSlider(IDC_SLIDER_NUM_SPOT_LIGHTS)->SetRange(0, maxSpotLights);
	//g_HUD.m_GUI.GetSlider(IDC_SLIDER_NUM_SPOT_LIGHTS)->SetValue(g_iNumActiveSpotLights);

	//g_NumPointLightsSlider->OnGuiEvent();
	//g_NumSpotLightsSlider->OnGuiEvent();
}