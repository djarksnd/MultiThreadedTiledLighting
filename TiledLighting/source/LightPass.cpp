#include <DirectXMath.h>

#include "LightPass.h"
#include "GeometryPass.h"
#include "ShadowDepthBuffer.h"
#include "TiledRenderer.h"
#include "Frustum.h"

// 16bytes align for cbuffer binding. 
#pragma pack(push, 16)
struct LightCullingCSCBuffer
{
    DirectX::XMMATRIX invProjectionMatrix;
    DirectX::XMMATRIX viewMatrix;
    float viewWidth;
    float viewHeight;
    uint32_t numPointLights;
    uint32_t numSpotLights;
    uint32_t numTilesX;
    uint32_t numTilesY;
    uint32_t padding[2];
};

struct DeferredLightingPSCBuffer
{
    DirectX::XMMATRIX projectionMatrix;
    DirectX::XMMATRIX invViewProjectionMatrix;
    DirectX::XMFLOAT3 cameraPosition;
    uint32_t numThreadsPerTileX;
    uint32_t numThreadsPerTileY;
    uint32_t numTilesX;
    uint32_t numPointLightShadows;
    uint32_t numSpotLightShadows;
    DirectX::XMMATRIX pointLightShadowMatrices[ShadowDepthBuffer::NumMaxPointLightShadows][6];
    DirectX::XMMATRIX spotLightShadowMatrices[ShadowDepthBuffer::NumMaxSpotLightShadows];
};
#pragma pack(pop)

bool LightPass::Create(const TiledRenderer& renderer)
{
    if (!Resize(renderer))
        return false;

    if (!pointLightBuffer.Create(renderer.GetDevice(), DXGI_FORMAT_UNKNOWN,
        sizeof(PointLight) * TiledRenderer::NumMaxPointLights, sizeof(PointLight),
        ResourceBindFlags::ShaderResourceBit, ResourceCPUAccessFlags::writeBit))
        return false;

    if (!spotLightBuffer.Create(renderer.GetDevice(), DXGI_FORMAT_UNKNOWN,
        sizeof(SpotLight) * TiledRenderer::NumMaxSpotLights, sizeof(SpotLight),
        ResourceBindFlags::ShaderResourceBit, ResourceCPUAccessFlags::writeBit))
        return false;

    if (!lightCullingCSCBuffer.Create(renderer.GetDevice(), sizeof(LightCullingCSCBuffer)))
        return false;

    std::vector<Shader::Macro> macros;
    macros.push_back({ "NumMaxPointLightsPerTile", std::to_string(NumMaxPointLightsPerTile) });
    macros.push_back({ "NumMaxSpotLightsPerTile", std::to_string(NumMaxSpotLightsPerTile) });
    macros.push_back({ "NumThreadsPerTileX", std::to_string(NumThreadsPerTileX) });
    macros.push_back({ "NumThreadsPerTileY", std::to_string(NumThreadsPerTileY) });
    macros.push_back({ "NumThreadsPerTile", std::to_string(NumThreadsPerTileX * NumThreadsPerTileY) });
    if (!lightCullingComputeShader.Create(renderer.GetDevice(), L"..\\media\\shader\\LightCullingComputeShader.hlsl", "main", macros))
        return false;

    macros.clear();
    macros.push_back({ "NumMaxPointLightsPerTile", std::to_string(NumMaxPointLightsPerTile) });
    macros.push_back({ "NumMaxSpotLightsPerTile", std::to_string(NumMaxSpotLightsPerTile) });
    macros.push_back({ "NumMaxPointLightShadows", std::to_string(ShadowDepthBuffer::NumMaxPointLightShadows) });
    macros.push_back({ "NumMaxSpotLightShadows", std::to_string(ShadowDepthBuffer::NumMaxSpotLightShadows) });
    macros.push_back({ "VisualizeNumLights", "0" });
    if (!deferredLightingPixelShader.Create(renderer.GetDevice(), L"..\\media\\shader\\DeferredLightingPixelShader.hlsl", "main", macros))
        return false;

    macros.clear();
    macros.push_back({ "NumMaxPointLightsPerTile", std::to_string(NumMaxPointLightsPerTile) });
    macros.push_back({ "NumMaxSpotLightsPerTile", std::to_string(NumMaxSpotLightsPerTile) });
    macros.push_back({ "NumMaxPointLightShadows", std::to_string(ShadowDepthBuffer::NumMaxPointLightShadows) });
    macros.push_back({ "NumMaxSpotLightShadows", std::to_string(ShadowDepthBuffer::NumMaxSpotLightShadows) });
    macros.push_back({ "VisualizeNumLights", "1" });
    if (!visualizeNumLightsPixelShader.Create(renderer.GetDevice(), L"..\\media\\shader\\DeferredLightingPixelShader.hlsl", "main", macros))
        return false;

    if (!deferredLightingPSCBuffer.Create(renderer.GetDevice(), sizeof(DeferredLightingPSCBuffer)))
        return false;

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = false;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = TiledRenderer::StenCilMask::DeferredLightable;
    dsDesc.StencilWriteMask = 0;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    if (!deferredLightingDepthStencilState.Create(renderer.GetDevice(), dsDesc))
        return false;

    D3D11_BLEND_DESC bdDesc;
    bdDesc.AlphaToCoverageEnable = false;
    bdDesc.IndependentBlendEnable = false;
    for (auto& rt : bdDesc.RenderTarget)
    {
        rt.BlendEnable = true;
        rt.SrcBlend = D3D11_BLEND_ONE;
        rt.DestBlend = D3D11_BLEND_ONE;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ZERO;
        rt.DestBlendAlpha = D3D11_BLEND_ONE;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    if (!deferredLightingBlendState.Create(renderer.GetDevice(), bdDesc))
        return false;

    D3D11_SAMPLER_DESC ssDesc;
    ssDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.MipLODBias = 0.0f;
    ssDesc.MaxAnisotropy = 1;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
    ssDesc.BorderColor[0] = 0.0f;
    ssDesc.BorderColor[1] = 0.0f;
    ssDesc.BorderColor[2] = 0.0f;
    ssDesc.BorderColor[3] = 0.0f;
    ssDesc.MinLOD = -FLT_MAX;
    ssDesc.MaxLOD = FLT_MAX;
    if (!shadowDepthSamplerState.Create(renderer.GetDevice(), ssDesc))
        return false;

    pointLights.reserve(TiledRenderer::NumMaxPointLights);
    spotLights.reserve(TiledRenderer::NumMaxSpotLights);

    return true;
}

bool LightPass::Resize(const TiledRenderer& renderer)
{
    lightIndexBuffer.Destroy();

    const unsigned int lightIndexBufferSize = GetNumTileX(renderer.GetScreenWidth()) *
        GetNumTileY(renderer.GetScreenHeight()) *
        ((NumMaxPointLightsPerTile + NumMaxSpotLightsPerTile) * 2 + 6)
        * sizeof(unsigned short);

    lightIndexBuffer.Create(renderer.GetDevice(),
        DXGI_FORMAT_R16_UINT,
        lightIndexBufferSize, sizeof(unsigned short),
        ResourceBindFlags::ShaderResourceBit | ResourceBindFlags::UnorderedAccessBit,
        0);

    return true;
}

void LightPass::Render(const TiledRenderer& renderer)
{
    renderer.PostRenderTask([this, &renderer]() {
        // Lights culling with compute shader and screen align tile's frustums.
        CullLights(renderer);

        // Calculate lighting with gbuffers and gpu lights culling results to scene color buffer.
        RenderLights(renderer);
        });
}

void LightPass::CullLights(const TiledRenderer& renderer)
{
    // Lights culling with compute shader and screen align tile's frustums.

    // cpu culling with camera frustum before gpu culling.
    pointLights.clear();
    spotLights.clear();

    const Frustum frustum(renderer.GetViewInfo().invViewProjectionMatrix);

    for (size_t index = 0; index < renderer.GetNumPointLightLimit(); ++index)
    {
        const PointLight& light = renderer.GetPointLight(index);
        if (frustum.CollisionCheck(DirectX::XMLoadFloat3(&light.position), light.radius))
            pointLights.push_back(light);
    }

    for (size_t index = 0; index < renderer.GetNumSpotLightLimit(); ++index)
    {
        const SpotLight& light = renderer.GetSpotLight(index);
        if (frustum.CollisionCheck(DirectX::XMLoadFloat3(&light.position), light.radius))
            spotLights.push_back(light);
    }

    ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();

    // cup culling results update.
    pointLightBuffer.Update(deviceContext,
        pointLights.data(),
        static_cast<unsigned int>(pointLights.size() * sizeof(PointLight)));

    spotLightBuffer.Update(deviceContext,
        spotLights.data(),
        static_cast<unsigned int>(spotLights.size() * sizeof(SpotLight)));

    // gpu culling with compute shader.
    ID3D11ShaderResourceView* srvs[] = {
        renderer.GetSceneDepthStencilBuffer(),
        pointLightBuffer,
        spotLightBuffer };

    deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    ID3D11UnorderedAccessView* uav = lightIndexBuffer;
    deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    LightCullingCSCBuffer cbufferData;
    cbufferData.viewMatrix = DirectX::XMMatrixTranspose(renderer.GetViewInfo().viewMatrix);
    cbufferData.invProjectionMatrix = DirectX::XMMatrixTranspose(renderer.GetViewInfo().invProjectionMatrix);
    cbufferData.viewWidth = static_cast<float>(renderer.GetScreenWidth());
    cbufferData.viewHeight = static_cast<float>(renderer.GetScreenHeight());
    cbufferData.numPointLights = static_cast<uint32_t>(pointLights.size());
    cbufferData.numSpotLights = static_cast<uint32_t>(spotLights.size());
    cbufferData.numTilesX = GetNumTileX(renderer.GetScreenWidth());
    cbufferData.numTilesY = GetNumTileY(renderer.GetScreenHeight());

    lightCullingCSCBuffer.Update(deviceContext, &cbufferData);

    ID3D11Buffer* buffer = lightCullingCSCBuffer;
    deviceContext->CSSetConstantBuffers(0, 1, &buffer);

    deviceContext->CSSetShader(lightCullingComputeShader, nullptr, 0);

    // light culling compute shader execution.
    deviceContext->Dispatch(GetNumTileX(renderer.GetScreenWidth()),
        GetNumTileY(renderer.GetScreenHeight()),
        1);

    for (auto& srv : srvs)
        srv = nullptr;

    uav = nullptr;
    buffer = nullptr;

    deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
    deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    deviceContext->CSSetConstantBuffers(0, 1, &buffer);
    deviceContext->CSSetShader(nullptr, nullptr, 0);
}

void LightPass::RenderLights(const TiledRenderer& renderer)
{
    // Calculate lighting with gbuffers and gpu lights culling results to scene color buffer.
    ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();

    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = static_cast<float>(renderer.GetScreenWidth());
    viewPort.Height = static_cast<float>(renderer.GetScreenHeight());
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewPort);

    ID3D11ShaderResourceView* srvs[] = {
        renderer.GetSceneDepthStencilBuffer(),
        renderer.GetGeometryPass().GetBuffer(GeometryPass::BufferType::DiffuseSpecular),
        renderer.GetGeometryPass().GetBuffer(GeometryPass::BufferType::NormalGlossiness),
        lightIndexBuffer,
        pointLightBuffer,
        spotLightBuffer,
        renderer.GetShadowDepthBuffer().GetPointLightShadowDepthBuffer(),
        renderer.GetShadowDepthBuffer().GetSpotLightShadowDepthBuffer() };

    deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    ID3D11SamplerState* sampler = shadowDepthSamplerState;
    deviceContext->PSSetSamplers(0, 1, &sampler);

    DeferredLightingPSCBuffer psCBufferData;
    psCBufferData.projectionMatrix = DirectX::XMMatrixTranspose(renderer.GetViewInfo().projectionMatrix);
    psCBufferData.invViewProjectionMatrix = DirectX::XMMatrixTranspose(renderer.GetViewInfo().invViewProjectionMatrix);
    DirectX::XMStoreFloat3(&psCBufferData.cameraPosition, renderer.GetViewInfo().viewOrigin);
    psCBufferData.numThreadsPerTileX = NumThreadsPerTileX;
    psCBufferData.numThreadsPerTileY = NumThreadsPerTileY;
    psCBufferData.numTilesX = GetNumTileX(renderer.GetScreenWidth());

    psCBufferData.numPointLightShadows =
        static_cast<uint32_t>(renderer.GetShadowDepthBuffer().GetNumCurrFramePointLightShadows());

    for (unsigned int shadowIndex = 0;
        shadowIndex < renderer.GetShadowDepthBuffer().GetNumCurrFramePointLightShadows();
        ++shadowIndex)
    {
        for (unsigned int face = 0; face < 6; ++face)
        {
            psCBufferData.pointLightShadowMatrices[shadowIndex][face] = ComputePointLightShadowMatrix(
                pointLights[shadowIndex], static_cast<PointLight::ShadowFace>(face));

            psCBufferData.pointLightShadowMatrices[shadowIndex][face] =
                DirectX::XMMatrixTranspose(psCBufferData.pointLightShadowMatrices[shadowIndex][face]);
        }
    }

    psCBufferData.numSpotLightShadows =
        static_cast<uint32_t>(renderer.GetShadowDepthBuffer().GetNumCurrFrameSpotLightShadows());

    for (unsigned int shadowIndex = 0;
        shadowIndex < renderer.GetShadowDepthBuffer().GetNumCurrFrameSpotLightShadows();
        ++shadowIndex)
    {
        psCBufferData.spotLightShadowMatrices[shadowIndex] = ComputeSpotLightShadowMatrix(spotLights[shadowIndex]);
        psCBufferData.spotLightShadowMatrices[shadowIndex] = DirectX::XMMatrixTranspose(psCBufferData.spotLightShadowMatrices[shadowIndex]);
    }

    deferredLightingPSCBuffer.Update(deviceContext, &psCBufferData);
    ID3D11Buffer* buffer = deferredLightingPSCBuffer;
    deviceContext->PSSetConstantBuffers(0, 1, &buffer);

    ID3D11RenderTargetView* rtv = renderer.GetSceneRenderTarget();

    if (renderer.GetVisualizeNumLights())
    {
        deviceContext->PSSetShader(visualizeNumLightsPixelShader, nullptr, 0);
        deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
    }
    else
    {
        deviceContext->PSSetShader(deferredLightingPixelShader, nullptr, 0);
        deviceContext->OMSetRenderTargets(1, &rtv, renderer.GetSceneDepthStencilBuffer().GetReadOnlyDepthStencilView());
        deviceContext->OMSetBlendState(deferredLightingBlendState, nullptr, 0xffffffff);
        deviceContext->OMSetDepthStencilState(deferredLightingDepthStencilState, TiledRenderer::StenCilMask::DeferredLightable);
    }

    renderer.DrawScreenAlignQuad();

    if (!renderer.GetEnableMultiThreadedRendering())
    {
        // restore d3d rendering states if not multi threaded rendering mode for next drawing steps.
        for (auto& srv : srvs)
            srv = nullptr;

        deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

        sampler = nullptr;
        deviceContext->PSSetSamplers(0, 1, &sampler);

        buffer = nullptr;
        deviceContext->PSSetConstantBuffers(0, 1, &buffer);
        deviceContext->PSSetShader(nullptr, nullptr, 0);

        rtv = nullptr;
        deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
        deviceContext->OMSetDepthStencilState(nullptr, 0);
        deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    }
}
