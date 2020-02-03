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

CDXUTDialogResourceManager dialogResourceManager;
CDXUTDialog hud;
std::shared_ptr<CDXUTTextHelper> textHelper;
std::shared_ptr<TiledRenderer> tiledRenderer;
CFirstPersonCamera camera;
float maxSceneDistance = 500.0f;

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
    hud.Init(&dialogResourceManager);
    hud.SetBackgroundColors(DlgColor);

    hud.SetCallback(OnGUIEvent);
    int iY = 10;
    hud.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 70, iY, 125, 22);

    hud.AddStatic(IDC_NumPointLight, L"NumPointLights :", 40, iY += 30, 125, 22);
    hud.AddSlider(IDC_NumPointLightSlide, 25, iY += 24, 200, 22, 0,
                         TiledRenderer::NumMaxPointLights, 0);

    hud.AddStatic(IDC_NumSpotLight, L"NumSpotLight :", 40, iY += 24, 125, 22);
    hud.AddSlider(IDC_NumSpotLightSlide, 25, iY += 24, 200, 22, 0,
                         TiledRenderer::NumMaxSpotLights, 0);

    hud.AddStatic(IDC_NumPointLightShadow, L"NumPointLightShadow :", 40, iY += 24, 125, 22);
    hud.AddSlider(IDC_NumPointLightShadowSlide, 25, iY += 24, 200, 22, 0,
                         ShadowDepthBuffer::NumMaxPointLightShadows, 0);

    hud.AddStatic(IDC_NumSpotLightShadow, L"NumSpotLightShadow :", 40, iY += 24, 125, 22);
    hud.AddSlider(IDC_NumSpotLightShadowSlide, 25, iY += 24, 200, 22, 0,
                         ShadowDepthBuffer::NumMaxSpotLightShadows, 0);

    iY += 24;
    hud.AddCheckBox(IDC_MultiThreadedRendering, L"MultiThreadedRendering", 20, iY += 24, 200, 22, false);

    iY += 24;
    hud.AddCheckBox(IDC_VisualizeNumLights, L"VisualizeNumLightsPerTile", 20, iY += 24, 200, 22, false);

    hud.AddStatic(IDC_Gamma, L"Gamma :", 40, iY += 40, 125, 22);
    hud.AddSlider(IDC_GammaSlide, 25, iY += 24, 200, 22, 0, 50, 0);

    UpdateGUI();
}


void RenderGUI(float elapsedTime)
{
    ID3D11DeviceContext* deviceContext = DXUTGetD3D11DeviceContext();
    ID3D11RenderTargetView* renderTargetView = DXUTGetD3D11RenderTargetView();

    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = static_cast<float>(tiledRenderer->GetScreenWidth());
    viewPort.Height = static_cast<float>(tiledRenderer->GetScreenHeight());
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewPort);

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
    hud.OnRender(elapsedTime);

    textHelper->Begin();
    textHelper->SetInsertionPos(5, 5);
    textHelper->SetForegroundColor(DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
    textHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
    textHelper->DrawTextLine(DXUTGetDeviceStats());
    textHelper->End();
}

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext)
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN(dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
    textHelper = std::shared_ptr<CDXUTTextHelper>(
        new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &dialogResourceManager, 15));

    tiledRenderer = std::shared_ptr<TiledRenderer>(new TiledRenderer);
    if (tiledRenderer)
    {
        ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        tiledRenderer->Create(pd3dDevice,
                           pd3dImmediateContext,
                           pBackBufferSurfaceDesc->Width,
                           pBackBufferSurfaceDesc->Height,
                           pBackBufferSurfaceDesc->SampleDesc.Count);

        using namespace DirectX;
        const XMVECTOR sceneMin = DirectX::XMLoadFloat3(&tiledRenderer->GetSceneBound().GetMin());
        const XMVECTOR sceneMax = DirectX::XMLoadFloat3(&tiledRenderer->GetSceneBound().GetMax());

        XMVECTOR sceneCenter = 0.5f * (sceneMax + sceneMin);
        XMVECTOR SceneExtents = 0.5f * (sceneMax - sceneMin);
        XMVECTOR boundaryMin = sceneCenter - 2.0f * SceneExtents;
        XMVECTOR boundaryMax = sceneCenter + 2.0f * SceneExtents;
        XMVECTOR boundaryDiff = boundaryMax - boundaryMin;
        maxSceneDistance = XMVectorGetX(XMVector3Length(boundaryDiff));

        XMVECTOR eye = sceneCenter - XMVectorSet(0.45f*XMVectorGetX(SceneExtents), 0.35f*XMVectorGetY(SceneExtents), 0.0f, 0.0f);
        XMVECTOR lookAt = sceneCenter - XMVectorSet(0.0f, 0.35f*XMVectorGetY(SceneExtents), 0.0f, 0.0f);
        camera.SetRotateButtons(true, false, false);
        camera.SetEnablePositionMovement(true);
        camera.SetViewParams(eye, lookAt);
        camera.SetScalers(0.005f, 0.1f*maxSceneDistance);
        XMFLOAT3 vboundaryMin, vboundaryMax;
        XMStoreFloat3(&vboundaryMin, boundaryMin);
        XMStoreFloat3(&vboundaryMax, boundaryMax);
        camera.SetClipToBoundary(true, &vboundaryMin, &vboundaryMax);

        UpdateGUI();
    }

    DXUTGetDXGIFactory()->MakeWindowAssociation(DXUTGetHWND(), DXGI_MWA_NO_ALT_ENTER);

    return S_OK;
}

HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    V_RETURN(dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

    if (tiledRenderer)
    {
        tiledRenderer->Resize(pBackBufferSurfaceDesc->Width,
                           pBackBufferSurfaceDesc->Height,
                           pBackBufferSurfaceDesc->SampleDesc.Count);
    }

    float aspectRatio = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);
    camera.SetProjParams(DirectX::XM_PI / 4.0f, aspectRatio, maxSceneDistance, 1.0f);

    hud.SetLocation(pBackBufferSurfaceDesc->Width - 250, 0);
    hud.SetSize(250, 400);

    return S_OK;
}

void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float elapsedTime, void* pUserContext)
{
    if (tiledRenderer)
        tiledRenderer->Render(camera);

    RenderGUI(elapsedTime);
}

void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    dialogResourceManager.OnD3D11ReleasingSwapChain();
}

void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    tiledRenderer.reset();

    dialogResourceManager.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();

    textHelper.reset();
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
    if (tiledRenderer)
        tiledRenderer->Update(elapsedTime);

    camera.FrameMove(elapsedTime);
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

    *pbNoFurtherProcessing = dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    *pbNoFurtherProcessing = hud.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    camera.HandleMessages(hWnd, uMsg, wParam, lParam);

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
        if (tiledRenderer)
        {
            const bool enable = hud.GetCheckBox(IDC_MultiThreadedRendering)->GetChecked();
            tiledRenderer->SetEnableMultiThreadedRendering(enable);
        }
        break;
    }
    case IDC_VisualizeNumLights:
    {
        if (tiledRenderer)
        {
            const bool enable = hud.GetCheckBox(IDC_VisualizeNumLights)->GetChecked();
            tiledRenderer->SetVisualizeNumLights(enable);
        }
        break;
    }
    case IDC_NumPointLightSlide:
    {
        if (tiledRenderer)
        {
            const size_t numLimit = hud.GetSlider(IDC_NumPointLightSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumPointLight : ") + std::to_wstring(numLimit);
            hud.GetStatic(IDC_NumPointLight)->SetText(text.c_str());
            tiledRenderer->SetNumPointLightLimit(numLimit);
        }
        break;
    }
    case IDC_NumSpotLightSlide:
    {
        if (tiledRenderer)
        {
            const size_t numLimit = hud.GetSlider(IDC_NumSpotLightSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumSpotLight : ") + std::to_wstring(numLimit);
            hud.GetStatic(IDC_NumSpotLight)->SetText(text.c_str());
            tiledRenderer->SetNumSpotLightLimit(numLimit);
        }
        break;
    }
    case IDC_NumPointLightShadowSlide:
    {
        if (tiledRenderer)
        {
            const size_t numLimit = hud.GetSlider(IDC_NumPointLightShadowSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumPointLightShadow : ") + std::to_wstring(numLimit);
            hud.GetStatic(IDC_NumPointLightShadow)->SetText(text.c_str());
            tiledRenderer->SetNumPointLightShadowLimit(numLimit);
        }
        break;
    }
    case IDC_NumSpotLightShadowSlide:
    {
        if (tiledRenderer)
        {
            const size_t numLimit = hud.GetSlider(IDC_NumSpotLightShadowSlide)->GetValue();
            const std::wstring text = std::wstring(L"NumSpotLightShadow : ") + std::to_wstring(numLimit);
            hud.GetStatic(IDC_NumSpotLightShadow)->SetText(text.c_str());
            tiledRenderer->SetNumSpotLightShadowLimit(numLimit);
        }
        break;
    }
    case IDC_GammaSlide:
    {
        if (tiledRenderer)
        {
            const int value = hud.GetSlider(IDC_GammaSlide)->GetValue();
            const float gamma = MathHelper::Lerp<float>(0.5f, 1.5f, static_cast<float>(value) / 50.0f);
            const std::wstring text = std::wstring(L"Gamma : ") + std::to_wstring(gamma);
            hud.GetStatic(IDC_Gamma)->SetText(text.c_str());
            tiledRenderer->SetGamma(gamma);
        }
        break;
    }
    }
}

void UpdateGUI()
{
    if (!tiledRenderer)
        return;

    {
        const int numLimit = static_cast<int>(tiledRenderer->GetNumPointLightLimit());
        const std::wstring text = std::wstring(L"NumPointLight : ") + std::to_wstring(numLimit);
        hud.GetStatic(IDC_NumPointLight)->SetText(text.c_str());
        hud.GetSlider(IDC_NumPointLightSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(tiledRenderer->GetNumSpotLightLimit());
        const std::wstring text = std::wstring(L"NumSpotLight : ") + std::to_wstring(numLimit);
        hud.GetStatic(IDC_NumSpotLight)->SetText(text.c_str());
        hud.GetSlider(IDC_NumSpotLightSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(tiledRenderer->GetShadowDepthBuffer().GetNumPointLightShadowLimit());
        const std::wstring text = std::wstring(L"NumPointLightShadow : ") + std::to_wstring(numLimit);
        hud.GetStatic(IDC_NumPointLightShadow)->SetText(text.c_str());
        hud.GetSlider(IDC_NumPointLightShadowSlide)->SetValue(numLimit);
    }

    {
        const int numLimit = static_cast<int>(tiledRenderer->GetShadowDepthBuffer().GetNumSpotLightShadowLimit());
        const std::wstring text = std::wstring(L"NumSpotLightShadow : ") + std::to_wstring(numLimit);
        hud.GetStatic(IDC_NumSpotLightShadow)->SetText(text.c_str());
        hud.GetSlider(IDC_NumSpotLightShadowSlide)->SetValue(numLimit);
    }

    {
        const bool enable = tiledRenderer->GetEnableMultiThreadedRendering();
        hud.GetCheckBox(IDC_MultiThreadedRendering)->SetChecked(enable);
    }

    {
        const bool enable = tiledRenderer->GetVisualizeNumLights();
        hud.GetCheckBox(IDC_VisualizeNumLights)->SetChecked(enable);
    }

    {
        const float gamma = tiledRenderer->GetGamma();
        const std::wstring text = std::wstring(L"Gamma : ") + std::to_wstring(gamma);
        hud.GetStatic(IDC_Gamma)->SetText(text.c_str());

        const int value = MathHelper::Min(static_cast<int>(ceil((gamma - 0.5f) * 50.0f)), 50);
        hud.GetSlider(IDC_GammaSlide)->SetValue(value);
    }
}
