#include <assert.h>
#include "Texture.h"




bool
Texture::Create(ID3D11Device* device, DXGI_FORMAT format,
				unsigned int width, unsigned int height, unsigned int arraySize,
				unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension,
				const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags)
{
	if (!CreateResource(device, format, width, height, arraySize, mipLevels, numSubSamples,
		dimension, bindFlags, cpuAccessFlags))
	{
		return false;
	}

	if (bindFlags.ShaderResource)
	{
		if (!CreateShaderResourceView(device, format, arraySize, mipLevels, numSubSamples, dimension))
		{
			return false;
		}
	}

	if (bindFlags.UnorderedAccess && (numSubSamples == 1))
	{
		if (!CreateUnorderedAccessView(device, format, arraySize, mipLevels, dimension))
		{
			return false;
		}
	}

	return true;
}


bool
Texture::CreateResource(ID3D11Device* device, DXGI_FORMAT format,
						unsigned int width, unsigned int height, unsigned int arraySize,
						unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension,
						const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags)
{
	unsigned int textureBindFlags = bindFlags.RenderTarget ? D3D11_BIND_RENDER_TARGET : 0;
	textureBindFlags |= bindFlags.DepthStencil ? D3D11_BIND_DEPTH_STENCIL : 0;
	textureBindFlags |= bindFlags.ShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0;
	textureBindFlags |= bindFlags.UnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0;

	unsigned int textureCPUAccessFlags = cpuAccessFlags.read ? D3D11_CPU_ACCESS_READ : 0;
	textureCPUAccessFlags |= cpuAccessFlags.write ? D3D11_CPU_ACCESS_WRITE : 0;

	D3D11_USAGE textureUsage;
	if (cpuAccessFlags.read)
		textureUsage = D3D11_USAGE_STAGING;
	else if (cpuAccessFlags.write)
		textureUsage = D3D11_USAGE_DYNAMIC;
	else
	{
		if (bindFlags.RenderTarget || bindFlags.DepthStencil || bindFlags.UnorderedAccess)
			textureUsage = D3D11_USAGE_DEFAULT;
		else
			textureUsage = D3D11_USAGE_IMMUTABLE;
	}

	unsigned int textureMiscFlags =
		(bindFlags.RenderTarget && (mipLevels > 1)) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	if (dimension == Dimension::Dimension1D)
	{
		D3D11_TEXTURE1D_DESC desc;
		desc.Width = width;
		desc.MipLevels = mipLevels;
		desc.ArraySize = arraySize;
		desc.Format = format;
		desc.Usage = textureUsage;
		desc.BindFlags = textureBindFlags;
		desc.CPUAccessFlags = textureCPUAccessFlags;
		desc.MiscFlags = textureMiscFlags;

		ID3D11Texture1D* texture1D = nullptr;
		HRESULT result = device->CreateTexture1D(&desc, nullptr, &texture1D);
		if (FAILED(result))
		{
			assert(false && "failed to CreateTexture2D.");
			return false;
		}

		resource = texture1D;
	}
	else if (dimension == Dimension::Dimension2D ||
			 dimension == Dimension::DimensionCube)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mipLevels;
		desc.ArraySize = arraySize;
		desc.Format = format;
		desc.SampleDesc.Count = (dimension == Dimension::Dimension2D) ? numSubSamples : 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = textureUsage;
		desc.BindFlags = textureBindFlags;
		desc.CPUAccessFlags = textureCPUAccessFlags;
		desc.MiscFlags = textureMiscFlags;

		if (dimension == Dimension::DimensionCube)
		{
			desc.ArraySize *= 6;
			desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		ID3D11Texture2D* texture2D = nullptr;
		HRESULT result = device->CreateTexture2D(&desc, nullptr, &texture2D);
		if (FAILED(result))
		{
			assert(false && "failed to CreateTexture2D.");
			return false;
		}

		resource = texture2D;
	}
	else if (dimension == Dimension::Dimension3D)
	{
		D3D11_TEXTURE3D_DESC desc;
		desc.Width = width;
		desc.Height = height;
		desc.Depth = arraySize;
		desc.MipLevels = mipLevels;
		desc.Format = format;
		desc.Usage = textureUsage;
		desc.BindFlags = textureBindFlags;
		desc.CPUAccessFlags = textureCPUAccessFlags;
		desc.MiscFlags = textureMiscFlags;

		ID3D11Texture3D* texture3D = nullptr;
		HRESULT result = device->CreateTexture3D(&desc, nullptr, &texture3D);
		if (FAILED(result))
		{
			assert(false && "failed to CreateTexture3D.");
			return false;
		}

		resource = texture3D;
	}

	return true;
}


bool
Texture::CreateUnorderedAccessView(ID3D11Device* device, DXGI_FORMAT format, unsigned int arraySize,
								   unsigned int mipLevels, Dimension dimension)
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

	if (dimension == Dimension::Dimension1D)
	{
		if (arraySize > 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.FirstArraySlice = 0;
			desc.Texture1DArray.ArraySize = arraySize;
		}
		else
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
	}
	else if (dimension == Dimension::Dimension2D)
	{
		if (arraySize > 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = arraySize;
		}
		else
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	}
	else if (dimension == Dimension::Dimension3D)
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.FirstWSlice = 0;
		desc.Texture3D.WSize = arraySize;
	}
	else if (dimension == Dimension::DimensionCube)
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = arraySize * 6;
	}

	unorderedAccessViews.resize(mipLevels);
	for (unsigned int i = 0; i < mipLevels; ++i)
	{
		if (dimension == Dimension::Dimension1D)
		{
			if (arraySize > 1)
				desc.Texture1DArray.MipSlice = i;
			else
				desc.Texture1D.MipSlice = i;
		}
		else if (dimension == Dimension::Dimension2D)
		{
			if (arraySize > 1)
				desc.Texture2DArray.MipSlice = i;
			else
				desc.Texture2D.MipSlice = i;
		}
		else if (dimension == Dimension::Dimension3D)
			desc.Texture3D.MipSlice = i;
		else if (dimension == Dimension::DimensionCube)
			desc.Texture2DArray.MipSlice = i;

		HRESULT result = device->CreateUnorderedAccessView(resource, &desc, &unorderedAccessViews[i]);
		if (FAILED(result))
		{
			assert(false && "Failed to Create ShaderResourceView.");
			return false;
		}
	}

	return true;
}


bool
Texture::CreateShaderResourceView(ID3D11Device* device, DXGI_FORMAT format, unsigned int arraySize,
								  unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension)
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

	if (dimension == Dimension::Dimension1D)
	{
		if (arraySize > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MostDetailedMip = 0;
			desc.Texture1DArray.MipLevels = mipLevels;
			desc.Texture1DArray.FirstArraySlice = 0;
			desc.Texture1DArray.ArraySize = arraySize;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture1D.MipLevels = mipLevels;
		}
	}
	else if (dimension == Dimension::Dimension2D)
	{
		if (numSubSamples > 1)
		{
			if (arraySize > 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = 0;
				desc.Texture2DMSArray.ArraySize = arraySize;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (arraySize > 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = 0;
				desc.Texture2DArray.MipLevels = mipLevels;
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.ArraySize = arraySize;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = 0;
				desc.Texture2D.MipLevels = mipLevels;
			}
		}
	}
	else if (dimension == Dimension::Dimension3D)
	{
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MostDetailedMip = 0;
		desc.Texture3D.MipLevels = mipLevels;
	}
	else if (dimension == Dimension::DimensionCube)
	{
		if (arraySize > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
			desc.TextureCubeArray.MostDetailedMip = 0;
			desc.TextureCubeArray.MipLevels = mipLevels;
			desc.TextureCubeArray.First2DArrayFace = 0;
			desc.TextureCubeArray.NumCubes = arraySize;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MostDetailedMip = 0;
			desc.TextureCube.MipLevels = mipLevels;
		}
	}

	HRESULT result = device->CreateShaderResourceView(resource, &desc, &shaderResourceView);
	if (FAILED(result))
	{
		assert(false && "Failed to Create ShaderResourceView.");
		return false;
	}

	return true;
}



void
Texture::Destroy()
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

	if (resource)
	{
		resource->Release();
		resource = nullptr;
	}
}



Texture::~Texture()
{
	for (auto& uav : unorderedAccessViews)
		uav->Release();

	if (shaderResourceView)
		shaderResourceView->Release();

	if (resource)
		resource->Release();
}