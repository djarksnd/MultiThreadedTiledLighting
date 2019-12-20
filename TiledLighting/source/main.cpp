#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Core\\DXUTmisc.h"
#include "..\\..\\DXUT\\Optional\\DXUTgui.h"
#include "..\\..\\DXUT\\Optional\\DXUTCamera.h"
#include "..\\..\\DXUT\\Optional\\SDKmisc.h"

#include "TiledRenderer.h"
#include "MathHelper.h"

enum IDC
{
    IDC_TOGGLEFULLSCREEN,
    IDC_MultiThreadedRendering,
    IDC_VisualizeNumLights,

    IDC_NumPointLight,
    IDC_NumPointLightSlide,

    IDC_NumSpotLight,
    IDC_NumSpotLightSlide,

    IDC_NumPointLightShadow,
    IDC_NumPointLightShadowSlide,

    IDC_NumSpotLightShadow,
    IDC_NumSpotLightShadowSlide,

    IDC_Gamma,
    IDC_GammaSlide,
};

CDXUTDialogResourceManager g_dialogResourceManager;
CDXUTDialog g_HUD;
std::shared_ptr<CDXUTTextHelper> g_textHelper;
std::shared_ptr<TiledRenderer> g_renderer;
CFirstPersonCamera g_camera;
float g_maxSceneDistance = 500.0f;

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float elapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float elapsedTime, void* pUserContext);

void InitializeGUI();
void RenderGUI(float elapsedTime);
void UpdateGUI();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    // Set DXUT callbacks
    DXUTSetCallbackMsgProc(MsgProc);
    DXUTSetCallbackFrameMove(OnFrameMove);
    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

    DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
    DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
    DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
    DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);

    InitializeGUI();
    DXUTInit(true, true, NULL);
    DXUTSetCursorSettings(true, true);
    DXUTCreateWindow(L"TiledRendering");
    DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1280, 800);

    SetFocus(DXUTGetHWND());
    DXUTMainLoop();

    return DXUTGetExitCode();
}

void InitializeGUI()
{
    D3DCOLOR DlgColor = 0x88888888;
    g_HUD.Init(&g_dialogResourceManager);
    g_HUD.SetBackgroundColors(DlgColor);

    g_HUD.SetCallback(OnGUIEvent);
    int iY = 10;
    g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 70, iY, 125, 22);

    g_HUD.AddStatic(IDC_NumPointLight, L"NumPointLights :", 40, iY += 30, 125, 22);
    g_HUD.AddSlider(IDC_NumPointLightSlide, 25, iY += 24, 200, 22, 0,
                         TiledRenderer::NumMaxPointLights, 0);

    g_HUD.AddStatic(IDC_NumSpotLight, L"NumSpotLight :", 40, iY += 24, 125, 22);
    g_HUD.AddSlider(IDC_NumSpotLightSlide, 25, iY += 24, 200, 22, 0,
                         TiledRenderer::NumMaxSpotLights, 0);

    g_HUD.AddStatic(IDC_NumPointLightShadow, L"NumPointLightShadow :", 40, iY += 24, 125, 22);
    g_HUD.AddSlider(IDC_NumPointLightShadowSlide, 25, iY += 24, 200, 22, 0,
                         ShadowDepthBuffer::NumMaxPointLightShadows, 0);

    g_HUD.AddStatic(IDC_NumSpotLightShadow, L"NumSpotLightShadow :", 40, iY += 24, 125, 22);
    g_HUD.AddSlider(IDC_NumSpotLightShadowSlide, 25, iY += 24, 200, 22, 0,
                         ShadowDepthBuffer::NumMaxSpotLightShadows, 0);

    iY += 24;
    g_HUD.AddCheckBox(IDC_MultiThreadedRendering, L"MultiThreadedRendering", 20, iY += 24, 200, 22, false);

    iY += 24;
    g_HUD.AddCheckBox(IDC_VisualizeNumLights, L"VisualizeNumLightsPerTile", 20, iY += 24, 200, 22, false);

    g_HUD.AddStatic(IDC_Gamma, L"Gamma :", 40, iY += 40, 125, 22);
    g_HUD.AddSlider(IDC_GammaSlide, 25, iY += 24, 200, 22, 0, 50, 0);

    UpdateGUI();
}


void RenderGUI(float elapsedTime)
{
    ID3D11DeviceContext* deviceContext = DXUTGetD3D11DeviceContext();
    ID3D11RenderTargetView* renderTargetView = DXUTGetD3D11RenderTargetView();

    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = static_cast<float>(g_renderer->GetScreenWidth());
    viewPort.Height = static_cast<float>(g_renderer->GetScreenHeight());
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewPort);

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
    g_HUD.OnRender(elapsedTime);

    g_textHelper->Begin();
    g_textHelper->SetInsertionPos(5, 5);
    g_textHelper->SetForegroundColor(DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
    g_textHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
    g_textHelper->DrawTextLine(DXUTGetDeviceStats());
    g_textHelper->End();
}

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext)
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN(g_dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
    g_textHelper = std::shared_ptr<CDXUTTextHelper>(
        new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15));

    g_renderer = std::shared_ptr<TiledRenderer>(new TiledRenderer);
    if (g_renderer)
    {
        ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        g_renderer->Create(pd3dDevice,
                           pd3dImmediateContext,
                           pBackBufferSurfaceDesc->Width,
                           pBackBufferSurfaceDesc->Height,
                           pBackBufferSurfaceDesc->SampleDesc.Count);

        using namespace DirectX;
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

        UpdateGUI();
    }

    DXUTGetDXGIFactory()->MakeWindowAssociation(DXUTGetHWND(), DXGI_MWA_NO_ALT_ENTER);

    return S_OK;
}

HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

    if (g_renderer)
    {
        g_renderer->Resize(pBackBufferSurfaceDesc->Width,
                           pBackBufferSurfaceDesc->Height,
                           pBackBufferSurfaceDesc->SampleDesc.Count);
    }

    float aspectRatio = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);
    g_camera.SetProjParams(DirectX::XM_PI / 4.0f, aspectRatio, g_maxSceneDistance, 1.0f);

    g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 250, 0);
    g_HUD.SetSize(250, 400);

    return S_OK;
}

void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float elapsedTime, void* pUserContext)
{
    if (g_renderer)
        g_renderer->Render(g_camera);

    RenderGUI(elapsedTime);
}

void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    g_renderer.reset();

    g_dialogResourceManager.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();

    g_textHelper.reset();
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

void CALLBACK OnFrameMove(double fTime, float elapsedTime, void* pUserContext)
{
    if (g_renderer)
        g_renderer->Update(elapsedTime);

    g_camera.FrameMove(elapsedTime);
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                         void* pUserContext)
{
    switch (uMsg)
    {
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 512;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 512;
        *pbNoFurtherProcessing = true;
        break;
    }

    *pbNoFurtherProcessing = g_dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    *pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);

    return 0;
}

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch (nControlID)
    {
    case IDC_TOGGLEFULLSCREEN:
        DXUTToggleFullScreen();
        break;
    case IDC_MultiThreadedRendering:
    {
        if (g_renderer)
        {
            const bool enable = g_HUD.GetCheckBox(IDC_MultiThreadedRendering)->GetChecked();
            g_renderer->SetEnableMultiThreadedRendering(enable);
        }
        break;
    }
    case IDC_VisualizeNumLights:
    {
        if (g_renderer)
        {
            const bool enable = g_HUD.GetCheckBox(IDC_VisualizeNumLights)->GetChecked();
            g_renderer->SetVisualizeNumLights(enable);
        }
        break;
    }
    case IDC_NumPointLightSlide:
    {
        if (g_renderer)
        {
            const size_t numLimit = g_HUD.GetSlider(IDC_NumPointLightSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumPointLight : ") + std::to_wstring(numLimit);
            g_HUD.GetStatic(IDC_NumPointLight)->SetText(text.c_str());
            g_renderer->SetNumPointLightLimit(numLimit);
        }
        break;
    }
    case IDC_NumSpotLightSlide:
    {
        if (g_renderer)
        {
            const size_t numLimit = g_HUD.GetSlider(IDC_NumSpotLightSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumSpotLight : ") + std::to_wstring(numLimit);
            g_HUD.GetStatic(IDC_NumSpotLight)->SetText(text.c_str());
            g_renderer->SetNumSpotLightLimit(numLimit);
        }
        break;
    }
    case IDC_NumPointLightShadowSlide:
    {
        if (g_renderer)
        {
            const size_t numLimit = g_HUD.GetSlider(IDC_NumPointLightShadowSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumPointLightShadow : ") + std::to_wstring(numLimit);
            g_HUD.GetStatic(IDC_NumPointLightShadow)->SetText(text.c_str());
            g_renderer->SetNumPointLightShadowLimit(numLimit);
        }
        break;
    }
    case IDC_NumSpotLightShadowSlide:
    {
        if (g_renderer)
        {
            const size_t numLimit = g_HUD.GetSlider(IDC_NumSpotLightShadowSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumSpotLightShadow : ") + std::to_wstring(numLimit);
            g_HUD.GetStatic(IDC_NumSpotLightShadow)->SetText(text.c_str());
            g_renderer->SetNumSpotLightShadowLimit(numLimit);
        }
        break;
    }
    case IDC_GammaSlide:
    {
        if (g_renderer)
        {
            const int value = g_HUD.GetSlider(IDC_GammaSlide)->GetValue();
            const float gamma = MathHelper::Lerp<float>(0.5f, 1.5f, static_cast<float>(value) / 50.0f);
            const std::wstring text = std::wstring(L"Gamma : ") + std::to_wstring(gamma);
            g_HUD.GetStatic(IDC_Gamma)->SetText(text.c_str());
            g_renderer->SetGamma(gamma);
        }
        break;
    }
    }
}

void UpdateGUI()
{
    if (!g_renderer)
        return;

    {
        const int numLimit = static_cast<int>(g_renderer->GetNumPointLightLimit());
        const std::wstring text = std::wstring(L"NumPointLight : ") + std::to_wstring(numLimit);
        g_HUD.GetStatic(IDC_NumPointLight)->SetText(text.c_str());
        g_HUD.GetSlider(IDC_NumPointLightSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(g_renderer->GetNumSpotLightLimit());
        const std::wstring text = std::wstring(L"NumSpotLight : ") + std::to_wstring(numLimit);
        g_HUD.GetStatic(IDC_NumSpotLight)->SetText(text.c_str());
        g_HUD.GetSlider(IDC_NumSpotLightSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(g_renderer->GetShadowDepthBuffer().GetNumPointLightShadowLimit());
        const std::wstring text = std::wstring(L"NumPointLightShadow : ") + std::to_wstring(numLimit);
        g_HUD.GetStatic(IDC_NumPointLightShadow)->SetText(text.c_str());
        g_HUD.GetSlider(IDC_NumPointLightShadowSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(g_renderer->GetShadowDepthBuffer().GetNumSpotLightShadowLimit());
        const std::wstring text = std::wstring(L"NumSpotLightShadow : ") + std::to_wstring(numLimit);
        g_HUD.GetStatic(IDC_NumSpotLightShadow)->SetText(text.c_str());
        g_HUD.GetSlider(IDC_NumSpotLightShadowSlide)->SetValue(numLimit);
    }

    {
        const bool enable = g_renderer->GetEnableMultiThreadedRendering();
        g_HUD.GetCheckBox(IDC_MultiThreadedRendering)->SetChecked(enable);
    }

    {
        const bool enable = g_renderer->GetVisualizeNumLights();
        g_HUD.GetCheckBox(IDC_VisualizeNumLights)->SetChecked(enable);
    }

    {
        const float gamma = g_renderer->GetGamma();
        const std::wstring text = std::wstring(L"Gamma : ") + std::to_wstring(gamma);
        g_HUD.GetStatic(IDC_Gamma)->SetText(text.c_str());

        const int value = MathHelper::Min(static_cast<int>(ceil((gamma - 0.5f) * 50.0f)), 50);
        g_HUD.GetSlider(IDC_GammaSlide)->SetValue(value);
    }
}
