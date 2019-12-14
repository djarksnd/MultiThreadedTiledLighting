#include <assert.h>
#include "Texture2D.h"




bool
Texture2D::Create(ID3D11Device* device, DXGI_FORMAT format,
				  unsigned int width, unsigned int height,
				  unsigned int mipLevels, unsigned int numSubSamples,
				  const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags)
{
	if (!CreateTrxture(device, format, width, height, mipLevels, numSubSamples, bindFlags, cpuAccessFlags))
	{
		return false;
	}

	if (bindFlags.ShaderResource)
	{
		if (!CreateShaderResourceView(device, format, mipLevels, numSubSamples))
		{
			return false;
		}
	}

	if (bindFlags.UnorderedAccess && (numSubSamples == 1))
	{
		if (!CreateUnorderedAccessView(device, format, mipLevels))
		{
			return false;
		}
	}

	return true;
}


bool 
Texture2D::CreateTrxture(ID3D11Device* device, DXGI_FORMAT format,
							  unsigned int width, unsigned int height,
							  unsigned int mipLevels, unsigned int numSubSamples,
							  const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mipLevels;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = numSubSamples;
	desc.SampleDesc.Quality = 0;
	
	desc.BindFlags = bindFlags.RenderTarget ? D3D11_BIND_RENDER_TARGET : 0;
	desc.BindFlags |= bindFlags.DepthStencil ? D3D11_BIND_DEPTH_STENCIL: 0;
	desc.BindFlags |= bindFlags.ShaderResource ? D3D11_BIND_SHADER_RESOURCE: 0;
	desc.BindFlags |= bindFlags.UnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS: 0;

	desc.CPUAccessFlags = cpuAccessFlags.read ? D3D11_CPU_ACCESS_READ : 0;
	desc.CPUAccessFlags |= cpuAccessFlags.write ? D3D11_CPU_ACCESS_WRITE : 0;

	// Usage.
	if (cpuAccessFlags.read)
	{
		desc.Usage = D3D11_USAGE_STAGING;
	}
	else if (cpuAccessFlags.write)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else
	{
		if (bindFlags.RenderTarget || bindFlags.DepthStencil || bindFlags.UnorderedAccess)
			desc.Usage = D3D11_USAGE_DEFAULT;
		else
			desc.Usage = D3D11_USAGE_IMMUTABLE;
	}

	// MiscFlags.
	if (bindFlags.RenderTarget && (1 != mipLevels))
	{
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else
	{
		desc.MiscFlags = 0;
	}

	HRESULT result = device->CreateTexture2D(&desc, nullptr, &texture);
	if (FAILED(result))
	{
		assert(false && "Failed to Create Texture2D.");
		return false;
	}

	return true;
}


bool 
Texture2D::CreateUnorderedAccessView(ID3D11Device* device, DXGI_FORMAT format, unsigned int mipLevels)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	switch (format)
	{
	case DXGI_FORMAT_R16_TYPELESS:
		desc.Format = DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_R32_TYPELESS:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	default:
		desc.Format = format;
		break;
	}

	desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

	unorderedAccessViews.resize(mipLevels);
	for (unsigned int i = 0; i < mipLevels; ++i)
	{
		desc.Texture2D.MipSlice = i;

		HRESULT result = device->CreateUnorderedAccessView(texture, &desc, &unorderedAccessViews[i]);
		if (FAILED(result))
		{
			assert(false && "Failed to Create ShaderResourceView.");
			return false;
		}
	}

	return true;
}


bool
Texture2D::CreateShaderResourceView(ID3D11Device* device, DXGI_FORMAT format, unsigned int mipLevels, unsigned int numSubSamples)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;

	switch (format)
	{
	case DXGI_FORMAT_R16_TYPELESS:
		desc.Format = DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_R32_TYPELESS:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	default:
		desc.Format = format;
		break;
	}

	if (1 < numSubSamples)
	{
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = mipLevels;
		desc.Texture2D.MostDetailedMip = 0;
	}

	HRESULT result = device->CreateShaderResourceView(texture, &desc, &shaderResourceView);
	if (FAILED(result))
	{
		assert(false && "Failed to Create ShaderResourceView.");
		return false;
	}

	return true;
}



void
Texture2D::Destroy()
{
	for (auto& uav : unorderedAccessViews)
	{
		uav->Release();
	}
	unorderedAccessViews.clear();

	if (shaderResourceView)
	{
		shaderResourceView->Release();
		shaderResourceView = nullptr;
	}

	if (texture)
	{
		texture->Release();
		texture = nullptr;
	}
}