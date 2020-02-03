#include <assert.h>
#include "DepthStencilBuffer.h"



bool DepthStencilBuffer::Create(
    ID3D11Device* device, Format format,
    unsigned int width, unsigned int height, unsigned int arraySize,
    unsigned int numSubSamples, Texture::Dimension dimension)
{
	DXGI_FORMAT textureFormat;
	switch (format)
	{
	case Format::Depth16:
		textureFormat = DXGI_FORMAT_R16_TYPELESS;
		break;
	case Format::Depth24_Stencil8:
		textureFormat = DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case Format::Depth32:
		textureFormat = DXGI_FORMAT_R32_TYPELESS;
		break;
	case Format::Depth32_Stencil8:
		textureFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	default:
		assert(false && "undefined format");
		return false;
	}

	if (!Texture::Create(device, textureFormat, width, height, arraySize, 1, numSubSamples, dimension,
		ResourceBindFlags::DepthStencilBit | ResourceBindFlags::ShaderResourceBit, 0))
		return false;

	if (!CreateDepthStencilViews(device, format, arraySize, numSubSamples, dimension))
		return false;

	return true;
}



bool
DepthStencilBuffer::CreateDepthStencilViews(
    ID3D11Device* device, Format format, unsigned int arraySize,
    unsigned int numSubSamples, Texture::Dimension dimension)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Flags = 0;

	switch (format)
	{
	case Format::Depth16:
		desc.Format = DXGI_FORMAT_D16_UNORM;
		break;
	case Format::Depth24_Stencil8:
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case Format::Depth32:
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		break;
	case Format::Depth32_Stencil8:
		desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		break;
	default:
		assert(false && "undefined format");
		return false;
	}

	if (dimension == Texture::Dimension::Dimension1D)
	{
		if (arraySize > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = 0;
			desc.Texture1DArray.FirstArraySlice = 0;
			desc.Texture1DArray.ArraySize = arraySize;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = 0;
		}
	}
	else if (dimension == Texture::Dimension::Dimension2D)
	{
		if (numSubSamples > 1)
		{
			if (arraySize > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = 0;
				desc.Texture2DMSArray.ArraySize = arraySize;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (arraySize > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = 0;
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.ArraySize = arraySize;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = 0;
			}
		}
	}
	else if (dimension == Texture::Dimension::Dimension3D)
	{
		assert(false && "3D texture can not create DepthStencilView.");
		return false;
	}
	else if (dimension == Texture::Dimension::DimensionCube)
	{
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = arraySize * 6;
	}

	HRESULT result = device->CreateDepthStencilView(*this, &desc, &depthStencilView);
	if (FAILED(result))
	{
		assert(false && "failed to Create DepthStencilView.");
		return false;
	}

	desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	if (format == Format::Depth24_Stencil8 || format == Format::Depth32_Stencil8)
		desc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;

	result = device->CreateDepthStencilView(*this, &desc, &readOnlyDepthStencilView);
	if (FAILED(result))
	{
		assert(false && "failed to Create ReadOnly DepthStencilView.");
		return false;
	}

	return true;
}



void DepthStencilBuffer::Clear(ID3D11DeviceContext* deviceContext, float depth, unsigned char stencil) const
{
	if (depthStencilView)
	{
		deviceContext->ClearDepthStencilView(depthStencilView,
											 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
}



DepthStencilBuffer::~DepthStencilBuffer()
{
	if (depthStencilView)
		depthStencilView->Release();

	if (readOnlyDepthStencilView)
		readOnlyDepthStencilView->Release();
}



void
DepthStencilBuffer::Destroy()
{
	if (depthStencilView)
	{
		depthStencilView->Release();
		depthStencilView = nullptr;
	}

	if (readOnlyDepthStencilView)
	{
		readOnlyDepthStencilView->Release();
		readOnlyDepthStencilView = nullptr;
	}

	Texture::Destroy();
}
