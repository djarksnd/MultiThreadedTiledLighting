#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <functional>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <vector>

#include "Object.h"
#include "ConstantBuffer.h"
#include "DepthStencilBuffer.h"
#include "RenderTarget.h"
#include "GeometryPass.h"
#include "LightPass.h"
#include "ScreenAlignQuad.h"
#include "PixelShader.h"
#include "ShadowDepthBuffer.h"
#include "Light.h"
#include "AABBox.h"

class CBaseCamera;

class TiledRenderer
{
public:
    static constexpr size_t NumMaxPointLights = 2048;
    static constexpr size_t NumMaxSpotLights = 2048;

    struct StenCilMask
    {
        enum Mask
        {
            DeferredLightable = 0x01,
        };
    };

    struct ViewInfo
    {
        DirectX::XMMATRIX viewMatrix;
        DirectX::XMMATRIX projectionMatrix;
        DirectX::XMMATRIX viewProjectionMatrix;
        DirectX::XMMATRIX invViewMatrix;
        DirectX::XMMATRIX invProjectionMatrix;
        DirectX::XMMATRIX invViewProjectionMatrix;
        DirectX::XMVECTOR viewOrigin;
    };

private:
    struct RenderingThread
    {
        std::thread thread;
        std::thread::id id;
        bool execution = false;
        ID3D11DeviceContext* deferredContext = nullptr;
    };

    struct RenderingTask
    {
        std::function<void()> task;
        ID3D11CommandList* commandList;
    };

public:
    TiledRenderer() {}
    ~TiledRenderer();

    bool Create(ID3D11Device* device, ID3D11DeviceContext* immediateContext,
                unsigned int screenWidth, unsigned int screenHeight, unsigned int numSubSamples);
    bool Resize(unsigned int screenWidth, unsigned int screenHeight, unsigned int numSubSamples);
    void Update(float elapsedTime);
    void Render(const CBaseCamera& camera);
    void AddObject(const std::shared_ptr<Object>& object);
    void RemoveObject(const std::shared_ptr<Object>& object);
    void DrawScreenAlignQuad() const;
    void PostRenderTask(const std::function<void()>& task) const;

    ID3D11Device* GetDevice() const {
        return device;
    }
    ID3D11DeviceContext* GetDeviceContext() const {
        const std::thread::id threadID = std::this_thread::get_id();
        for (auto& thread : threads)
        {
            if (thread.id == threadID)
                return thread.deferredContext;
        }

        return immediateContext;
    }
    unsigned int GetScreenWidth() const {
        return screenWidth;
    }
    unsigned int GetScreenHeight() const {
        return screenHeight;
    }
    unsigned int GetNumSubSamples() const {
        return numSubSamples;
    }
    void SetVisualizeNumLights(bool enable) {
        visualizeNumLights = enable;
    }
    bool GetVisualizeNumLights() const {
        return visualizeNumLights;
    }
    void SetEnableMultiThreadedRendering(bool enable) {
        enableMultiThreadedRendering = enable;
    }
    bool GetEnableMultiThreadedRendering() const {
        return enableMultiThreadedRendering;
    }
    void SetNumPointLightLimit(size_t num) {
        numPointLightLimit = num;
    }
    float GetGamma() const {
        return gamma;
    }
    void SetGamma(float value) {
        gamma = value;
    }
    size_t GetNumPointLightLimit() const {
        return numPointLightLimit;
    }
    void SetNumSpotLightLimit(size_t num) {
        numSpotLightLimit = num;
    }
    size_t GetNumSpotLightLimit() const {
        return numSpotLightLimit;
    }
    const RenderTarget& GetSceneRenderTarget() const {
        return sceneRenderTarget;
    }
    const DepthStencilBuffer& GetSceneDepthStencilBuffer() const {
        return sceneDepthStencilBuffer;
    }
    const GeometryPass& GetGeometryPass() const {
        return geometryPass;
    }
    const ShadowDepthBuffer& GetShadowDepthBuffer() const {
        return shadowDepthBuffer;
    }
    const std::vector<std::shared_ptr<Object>>& GetOpaqueObjects() const {
        return opaqueObjects;
    }
    const std::vector<std::shared_ptr<Object>>& GetMaskedObjects() const {
        return maskedObjects;
    }
    const PointLight& GetPointLight(size_t index) const {
        return pointLights[index];
    }
    const SpotLight& GetSpotLight(size_t index) const {
        return spotLights[index];
    }
    const ViewInfo& GetViewInfo() const {
        return viewInfo;
    }
    const AABBox& GetBound() const {
        return bound;
    }
    void SetNumPointLightShadowLimit(size_t num) {
        shadowDepthBuffer.SetNumPointLightShadowLimit(num);
    }
    void SetNumSpotLightShadowLimit(size_t num) {
        shadowDepthBuffer.SetNumSpotLightShadowLimit(num);
    }

private:
    void InitObjects();
    void InitLights();
    void RenderPostprocess() const;
    void FlushRenderTasks();

    static void RenderingThreadProc(RenderingThread& argThread, TiledRenderer& renderer);

private:
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* immediateContext;
    unsigned int screenWidth = 0;
    unsigned int screenHeight = 0;
    unsigned int numSubSamples = 0;
    float gamma = 1.0f;
    bool visualizeNumLights = false;
    RenderTarget sceneRenderTarget;
    DepthStencilBuffer sceneDepthStencilBuffer;

    GeometryPass geometryPass;
    ShadowDepthBuffer shadowDepthBuffer;
    LightPass lightPass;

    PixelShader postprocessPixelShader;
    ConstantBuffer postprocessCBuffer;
    ScreenAlignQuad screenAlignQuad;
    std::vector<std::shared_ptr<Object>> opaqueObjects;
    std::vector<std::shared_ptr<Object>> maskedObjects;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    size_t numPointLightLimit = NumMaxPointLights / 2;
    size_t numSpotLightLimit = NumMaxSpotLights / 2;
    ViewInfo viewInfo;
    AABBox bound;

    mutable std::vector<RenderingTask> renderingTasks;
    std::vector<RenderingThread> threads;
    std::shared_mutex sharedMutex;
    std::unique_lock<std::shared_mutex> mutexLock;
    std::atomic_int taskIndex;

    bool enableMultiThreadedRendering = true;
};
