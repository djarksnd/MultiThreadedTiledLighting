#include <assert.h>
#include "RenderTarget.h"



bool RenderTarget::Create(
    ID3D11Device* device, DXGI_FORMAT format,
    const ResourceBindFlags& bindFlags,
    unsigned int width, unsigned int height,
    unsigned int mipLevels, unsigned int numSubSamples)
{
    ResourceBindFlags newBindFlags = bindFlags;
    if (!newBindFlags.RenderTarget)
        newBindFlags.RenderTarget = true;

    if (!Texture::Create(device, format, width, height, 1, mipLevels, numSubSamples,
        Texture::Dimension::Dimension2D, newBindFlags, 0))
        return false;

    if (!CreateRenderTargetViews(device, format, mipLevels, numSubSamples))
    {
        return false;
    }

    return true;
}


bool RenderTarget::CreateRenderTargetViews(
    ID3D11Device* device, DXGI_FORMAT format,
    unsigned int mipLevels, unsigned int numSubSamples)
{
    renderTargetViews.resize(mipLevels);

    for (unsigned int i = 0; i < mipLevels; ++i)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = format;
        if (1 < numSubSamples)
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            rtvDesc.Texture2DMS.UnusedField_NothingToDefine = 0;
        }
        else
        {
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = i;
        }

        HRESULT result = device->CreateRenderTargetView(*this, &rtvDesc, &renderTargetViews[i]);
        if (FAILED(result))
        {
            assert(false && "failed to CreateRenderTargetView.");
            return false;
        }
    }

    return true;
}



void RenderTarget::Clear(ID3D11DeviceContext* deviceContext, const float color[4]) const
{
    for (auto& rtv : renderTargetViews)
    {
        deviceContext->ClearRenderTargetView(rtv, color);
    }
}



void RenderTarget::Destroy()
{
    for (auto& rtv : renderTargetViews)
    {
        rtv->Release();
    }

    renderTargetViews.clear();

    Texture::Destroy();
}



RenderTarget::~RenderTarget()
{
    for (auto& rtv : renderTargetViews)
    {
        rtv->Release();
    }

    renderTargetViews.clear();
}
