#include <windows.h>
#include <random>
#include <math.h>

#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Optional\\DXUTCamera.h"

#include "TiledRenderer.h"
#include "MathHelper.h"

// 16bytes align for cbuffer binding. 
#pragma pack(push, 16)
struct PostprocessCBuffer
{
    float invGamma;
    float padding[3];
};
#pragma pack(pop)

DWORD CountSetBits(ULONG_PTR bitMask) {

    const DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

    for (DWORD i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;

    }

    return bitSetCount;

}

size_t GetLogicalProcessorCount()
{
    DWORD numLogicalProcessors = 0;

    char* buffer = NULL;
    DWORD len = 0;

    if (FALSE == GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            buffer = new char[len];
            if (GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len))
            {
                char* ptr = buffer;
                while (ptr < buffer + len)
                {
                    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pi = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)ptr;
                    if (pi->Relationship == RelationProcessorCore)
                    {
                        for (size_t g = 0; g < pi->Processor.GroupCount; ++g)
                        {
                            numLogicalProcessors += CountSetBits(pi->Processor.GroupMask[g].Mask);
                        }
                    }
                    ptr += pi->Size;
                }
            }
            delete[] buffer;
        }
    }

    return static_cast<size_t>(numLogicalProcessors);
}

bool TiledRenderer::Create(ID3D11Device* argDevice, ID3D11DeviceContext* argImmediateContext,
                           unsigned int argScreenWidth, unsigned int argScreenHeight, unsigned int argNumSubSamples)
{
    device = argDevice;
    device->AddRef();

    immediateContext = argImmediateContext;
    immediateContext->AddRef();

    const size_t NumRenderingThreads = GetLogicalProcessorCount();
    if (NumRenderingThreads > 0)
    {
        threads.resize(NumRenderingThreads);
        for (auto& thread : threads)
        {
            if (FAILED(device->CreateDeferredContext(0, &thread.deferredContext)))
                return false;

            thread.run = true;
            thread.thread = std::thread(RenderingThreadProc, std::ref(thread), std::ref(*this));
            thread.id = thread.thread.get_id();
        }
    }

    screenWidth = argScreenWidth;
    screenHeight = argScreenHeight;
    numSubSamples = argNumSubSamples;

    if (!sceneRenderTarget.Create(device,
                                  DXGI_FORMAT_R11G11B10_FLOAT,
                                  ResourceBindFlags::RenderTargetBit | ResourceBindFlags::ShaderResourceBit,
                                  screenWidth,
                                  screenHeight,
                                  1,
                                  numSubSamples))
        return false;

    if (!sceneDepthStencilBuffer.Create(device,
                                        DepthStencilBuffer::Format::Depth24_Stencil8,
                                        screenWidth,
                                        screenHeight,
                                        1,
                                        numSubSamples,
                                        Texture::Dimension::Dimension2D))
        return false;

    if (!postprocessPixelShader.Create(device, L"..\\media\\shader\\PostprocessPixelShader.hlsl", "main"))
        return false;

    if (!postprocessCBuffer.Create(device, sizeof(PostprocessCBuffer)))
        return false;

    if (!screenAlignQuad.Create(device))
        return false;

    InitObjects();
    InitLights();

    if (!geometryPass.Create(*this))
        return false;

    if (!shadowDepthBuffer.Create(*this, 512))
        return false;

    if (!lightPass.Create(*this))
        return false;

    return true;
}

void TiledRenderer::InitObjects()
{
    std::shared_ptr<Object> mainSceneObject(new Object);
    mainSceneObject->Create(device, GetDeviceContext(), L"sponza\\sponza.sdkmesh");
    opaqueObjects.push_back(mainSceneObject);

    std::shared_ptr<Object> decorationObject(new Object);
    decorationObject->Create(device, GetDeviceContext(), L"sponza\\sponza_alpha.sdkmesh");
    maskedObjects.push_back(decorationObject);

    for (auto& object : opaqueObjects)
        sceneBound += object->GetBound();

    for (auto& object : maskedObjects)
        sceneBound += object->GetBound();
}

void TiledRenderer::InitLights()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1024);

    pointLights.resize(NumMaxPointLights);
    unsigned int index = 0;
    for (auto& light : pointLights)
    {
        if (index < 2)
        {
            light.position.x = 0.0f;
            light.position.y = 70.0f;

            if (index)
                light.position.z = 210.0f;
            else
                light.position.z = -140.0f;

            light.color.x = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.y = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.z = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.radius = 600.0f;
            light.falloff = 1.0f;
        }
        else
        {
            light.position.x = MathHelper::Lerp(sceneBound.GetMin().x, sceneBound.GetMax().x, static_cast<float>(dist(gen)) / 1024.0f);
            light.position.y = MathHelper::Lerp(sceneBound.GetMin().y, sceneBound.GetMax().y, static_cast<float>(dist(gen)) / 1024.0f);
            light.position.z = MathHelper::Lerp(sceneBound.GetMin().z, sceneBound.GetMax().z, static_cast<float>(dist(gen)) / 1024.0f);
            light.radius = MathHelper::Lerp(100.0f, 150.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.x = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.y = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.z = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.falloff = MathHelper::Lerp(1.0f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
        }

        ++index;
    }

    spotLights.resize(NumMaxSpotLights);
    index = 0;
    for (auto& light : spotLights)
    {
        if (index < ShadowDepthBuffer::NumMaxSpotLightShadows)
        {
            light.position.x = 0.0f;
            light.position.y = 0.0f;
            light.position.z = 0.0f;
            light.color.x = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.y = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.z = MathHelper::Lerp(0.15f, 1.5f, static_cast<float>(dist(gen)) / 1024.0f);
            light.falloff = 1.0f;
            light.direction.x = 0.0f;
            light.direction.y = 0.0f;
            light.direction.z = 1.0f;

            if (index % 2)
            {
                light.radius = 1000.0f;
                light.cosHalfAngle = cosf(DirectX::XMConvertToRadians(35.0f));
            }
            else
            {
                light.radius = 3000.0f;
                light.cosHalfAngle = cosf(DirectX::XMConvertToRadians(12.5f));
            }
        }
        else
        {
            light.position.x = MathHelper::Lerp(sceneBound.GetMin().x, sceneBound.GetMax().x, static_cast<float>(dist(gen)) / 1024.0f);
            light.position.y = MathHelper::Lerp(sceneBound.GetMin().y, sceneBound.GetMax().y, static_cast<float>(dist(gen)) / 1024.0f);
            light.position.z = MathHelper::Lerp(sceneBound.GetMin().z, sceneBound.GetMax().z, static_cast<float>(dist(gen)) / 1024.0f);
            light.radius = MathHelper::Lerp(100.0f, 200.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.x = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.y = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.color.z = MathHelper::Lerp(0.15f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.falloff = MathHelper::Lerp(1.0f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.direction.x = MathHelper::Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.direction.y = MathHelper::Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            light.direction.z = MathHelper::Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
            DirectX::XMStoreFloat3(&light.direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&light.direction)));
            light.cosHalfAngle = cosf(DirectX::XMConvertToRadians(22.5f));
        }

        ++index;
    }
}

void TiledRenderer::RenderPostprocess() const
{
    PostRenderTask([this]() {
        ID3D11DeviceContext* deviceContext = GetDeviceContext();

        D3D11_VIEWPORT viewPort;
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        viewPort.Width = static_cast<float>(screenWidth);
        viewPort.Height = static_cast<float>(screenHeight);
        viewPort.MinDepth = 0.0f;
        viewPort.MaxDepth = 1.0f;
        GetDeviceContext()->RSSetViewports(1, &viewPort);

        PostprocessCBuffer cbufferData;
        cbufferData.invGamma = 1.0f / gamma;
        postprocessCBuffer.Update(deviceContext, &cbufferData);
        ID3D11Buffer* cbuffer = postprocessCBuffer;
        deviceContext->PSSetConstantBuffers(0, 1, &cbuffer);

        ID3D11ShaderResourceView* srv = GetSceneRenderTarget();
        deviceContext->PSSetShaderResources(0, 1, &srv);

        deviceContext->PSSetShader(postprocessPixelShader, nullptr, 0);

        ID3D11RenderTargetView* rtv = DXUTGetD3D11RenderTargetView();
        deviceContext->OMSetRenderTargets(1, &rtv, nullptr);

        DrawScreenAlignQuad();

        cbuffer = nullptr;
        deviceContext->PSSetConstantBuffers(0, 1, &cbuffer);

        srv = nullptr;
        deviceContext->PSSetShaderResources(0, 1, &srv);
        deviceContext->PSSetShader(nullptr, nullptr, 0);

        rtv = nullptr;
        deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
                   });
}

TiledRenderer::~TiledRenderer()
{
    {
        std::lock_guard<std::shared_mutex> lock(sharedMutex);
        for (auto& thread : threads)
            thread.run = false;
    }

    conditionVariableToWakeUpRenderingThreads.notify_all();

    for (auto& thread : threads)
    {
        thread.thread.join();

        if (thread.deferredContext)
            thread.deferredContext->Release();
    }

    if (immediateContext)
        immediateContext->Release();

    if (device)
        device->Release();
}

void TiledRenderer::Update(float elapsedTime)
{
    // move lights
    const unsigned int numTrackItems = 8;
    static const DirectX::XMVECTORF32 track[2][numTrackItems] =
    {
        {
            { -1250.0f, 260.0f, -400.0f, 0.0f },
            { -1250.0f, 260.0f,  400.0f, 0.0f },
            {  -450.0f, 260.0f,  400.0f, 0.0f },
            {   350.0f, 260.0f,  400.0f, 0.0f },
            {  1150.0f, 260.0f,  400.0f, 0.0f },
            {  1150.0f, 260.0f, -400.0f, 0.0f },
            {   350.0f, 260.0f, -400.0f, 0.0f },
            {  -450.0f, 260.0f, -400.0f, 0.0f }
        },
        {
            { -1250.0f, 650.0f,  400.0f, 0.0f },
            { -1250.0f, 650.0f, -400.0f, 0.0f },
            {  -450.0f, 650.0f, -400.0f, 0.0f },
            {   350.0f, 650.0f, -400.0f, 0.0f },
            {  1150.0f, 650.0f, -400.0f, 0.0f },
            {  1150.0f, 650.0f,  400.0f, 0.0f },
            {   350.0f, 650.0f,  400.0f, 0.0f },
            {  -450.0f, 650.0f,  400.0f, 0.0f }
        }
    };

    static float trackTime = 0.0;
    trackTime = fmod(trackTime + (elapsedTime * 0.25f), static_cast<float>(numTrackItems));

    DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, elapsedTime * 0.5f, 0.0f);

    for (std::size_t index = 0; index < pointLights.size(); ++index)
    {
        if (index >= 2)
        {
            DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pointLights[index].position);
            if (index % 2)
                position = DirectX::XMVector3Rotate(position, rotation);
            else
                position = DirectX::XMVector3InverseRotate(position, rotation);

            DirectX::XMStoreFloat3(&pointLights[index].position, position);
        }
    }

    for (std::size_t index = 0; index < spotLights.size(); ++index)
    {
        if (index < ShadowDepthBuffer::NumMaxSpotLightShadows)
        {
            unsigned int currItem = static_cast<unsigned int>(trackTime + index);
            currItem = currItem % numTrackItems;
            unsigned int nextItem = (currItem + 1) % numTrackItems;

            const size_t trackIndex = (index % 2) ? 0 : 1;
            const DirectX::XMVECTOR position = DirectX::XMVectorLerp(track[trackIndex][currItem],
                                                                     track[trackIndex][nextItem],
                                                                     trackTime - floor(trackTime));

            DirectX::XMStoreFloat3(&spotLights[index].position, position);
            spotLights[index].direction = spotLights[index].position;

            if ((index % 2) == 0)
                spotLights[index].direction.y = 0.0f;

            DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&spotLights[index].direction);
            direction = DirectX::XMVectorScale(direction, -1.0f);
            direction = DirectX::XMVector3Normalize(direction);
            DirectX::XMStoreFloat3(&spotLights[index].direction, direction);
        }
        else
        {
            DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&spotLights[index].position);
            if (index % 2)
                position = DirectX::XMVector3Rotate(position, rotation);
            else
                position = DirectX::XMVector3InverseRotate(position, rotation);

            DirectX::XMStoreFloat3(&spotLights[index].position, position);
        }
    }
}

bool TiledRenderer::Resize(unsigned int argScreenWidth, unsigned int argScreenHeight, unsigned int argNumSubSamples)
{
    screenWidth = argScreenWidth;
    screenHeight = argScreenHeight;
    numSubSamples = argNumSubSamples;

    sceneRenderTarget.Destroy();
    if (!sceneRenderTarget.Create(device,
                                  DXGI_FORMAT_R11G11B10_FLOAT,
                                  ResourceBindFlags::RenderTargetBit | ResourceBindFlags::ShaderResourceBit,
                                  screenWidth,
                                  screenHeight,
                                  1,
                                  numSubSamples))
        return false;

    sceneDepthStencilBuffer.Destroy();
    if (!sceneDepthStencilBuffer.Create(device,
                                        DepthStencilBuffer::Format::Depth24_Stencil8,
                                        screenWidth,
                                        screenHeight,
                                        1,
                                        numSubSamples,
                                        Texture::Dimension::Dimension2D))
        return false;

    if (!geometryPass.Resize(*this))
        return false;

    if (!lightPass.Resize(*this))
        return false;

    return true;
}

void TiledRenderer::DrawScreenAlignQuad() const
{
    screenAlignQuad.Render(GetDeviceContext());
}

void TiledRenderer::PostRenderTask(const std::function<void()>& task) const
{
    if (enableMultiThreadedRendering)
    {
        std::lock_guard<std::shared_mutex> lock(sharedMutex);
        renderingTasks.push_back(RenderingTask{ nullptr, task });
    }
    else
        task();
}

void TiledRenderer::Render(const CBaseCamera& camera)
{
    viewInfo.viewMatrix = camera.GetViewMatrix();
    viewInfo.projectionMatrix = camera.GetProjMatrix();
    viewInfo.viewProjectionMatrix = viewInfo.viewMatrix * viewInfo.projectionMatrix;
    viewInfo.invViewMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.viewMatrix);
    viewInfo.invProjectionMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.projectionMatrix);
    viewInfo.invViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.viewProjectionMatrix);
    viewInfo.viewOrigin = camera.GetEyePt();

    // Clear scene color buffer and scene depth buffer.
    float clearColor[4] = { 0.0f };
    sceneRenderTarget.Clear(GetDeviceContext(), clearColor);
    sceneDepthStencilBuffer.Clear(GetDeviceContext(), 0.0f, 0);

    // Write geometry data of objects to geometry buffer.
    geometryPass.Render(*this);

    // Draw shadow depth before performing drawing lights.
    shadowDepthBuffer.Render(*this);

    // Draw lights using geometry buffers and shadow depth buffer.
    // This function also uses the compute shader to culling the lights before drawing them.
    lightPass.Render(*this);

    // Perform gamma operation and print the scene color to the back buffer.
    RenderPostprocess();

    // Run registered rendering tasks using multithreading and wait for all rendering tests to complete.
    FlushRenderTasks();
}

void TiledRenderer::FlushRenderTasks()
{
    taskIndex.store(0);
    numCompletedRenderingTasks.store(0);

    conditionVariableToWakeUpRenderingThreads.notify_all();

    std::unique_lock<std::shared_mutex> lock(sharedMutex);
    conditionVariableToWaitRenderingThreads.wait(lock,[this] {return (numCompletedRenderingTasks.load() == renderingTasks.size()); });

    // Execute the recorded command list after all rendering tasks are completed.
    for (auto& task : renderingTasks)
    {
        if (task.commandList)
        {
            GetDeviceContext()->ExecuteCommandList(task.commandList, false);
            task.commandList->Release();
        }
    }

    renderingTasks.clear();
}

void TiledRenderer::RenderingThreadProc(RenderingThread& thread, TiledRenderer& renderer)
{
    while (thread.run)
    {
        std::shared_lock<std::shared_mutex> sharedLock(renderer.sharedMutex);
        renderer.conditionVariableToWakeUpRenderingThreads.wait(sharedLock);

        while (true)
        {
            const int taskIndex = renderer.taskIndex++;
            if (taskIndex < renderer.renderingTasks.size())
            {
                // I use stack object for notify.
                // It prevent main thread blocking when rendering threads crashed during running rendering task.
                AutoRenderingTaskCompletionNotifier notifier(renderer);

                // Run rendering task and record command list of the rendering task.
                RenderingTask& task = renderer.renderingTasks[taskIndex];
                task.task();
                renderer.GetDeviceContext()->FinishCommandList(false, &task.commandList);
            }
            else
            {
                break;
            }
        }
    }
}
