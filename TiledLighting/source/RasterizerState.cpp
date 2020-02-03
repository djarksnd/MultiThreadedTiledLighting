#include <assert.h>
#include "RasterizerState.h"



bool RasterizerState::Create(ID3D11Device* device, const D3D11_RASTERIZER_DESC& desc)
{
    HRESULT result = device->CreateRasterizerState(&desc, &rasterizerState);
    if (FAILED(result))
    {
        assert(false && "Failed to create RasterizerState");
        return false;
    }

    return true;
}



bool RasterizerState::Create(
    ID3D11Device* device, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode,
    bool clockwise, bool multiSample, bool antialiasedLine, int depthBias, float depthBiasClamp,
    float slopeScaledDepthBias, bool depthClip, bool scissor)
{
    D3D11_RASTERIZER_DESC desc;
    desc.FillMode = fillMode;
    desc.CullMode = cullMode;
    desc.FrontCounterClockwise = clockwise;
    desc.DepthBias = depthBias;
    desc.DepthBiasClamp = depthBiasClamp;
    desc.SlopeScaledDepthBias = slopeScaledDepthBias;
    desc.DepthClipEnable = depthClip;
    desc.ScissorEnable = scissor;
    desc.MultisampleEnable = multiSample;
    desc.AntialiasedLineEnable = antialiasedLine;

    return Create(device, desc);
}



void RasterizerState::Destroy()
{
    if (rasterizerState)
    {
        rasterizerState->Release();
        rasterizerState = nullptr;
    }
}
