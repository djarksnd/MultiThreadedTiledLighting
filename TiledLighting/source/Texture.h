#pragma once

#include <d3d11.h>
#include <DXGIFormat.h>
#include <vector>

#include "ResourceBindFlags.h"
#include "ResourceCPUAccessFlags.h"

class Texture
{
public:
	enum class Dimension
	{
		Dimension1D = 1,
		Dimension2D,
		Dimension3D,
		DimensionCube,
	};

public:
	bool Create(ID3D11Device* device, DXGI_FORMAT format,
				unsigned int width, unsigned int height, unsigned int arraySize,
				unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension,
				const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags);

	virtual void Destroy();

	operator ID3D11Resource* () const {
		return resource;
	}
	operator ID3D11ShaderResourceView* () const {
		return shaderResourceView;
	}
	operator ID3D11UnorderedAccessView* () const {
		return unorderedAccessViews[0];
	}
	ID3D11UnorderedAccessView* GetUAV(unsigned int mipLevel) const {
		return unorderedAccessViews[mipLevel];
	}

private:
	bool CreateResource(ID3D11Device* device, DXGI_FORMAT format,
						unsigned int width, unsigned int height, unsigned int arraySize,
						unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension,
						const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags);

	bool CreateShaderResourceView(ID3D11Device* device, DXGI_FORMAT format, unsigned int arraySize,
								  unsigned int mipLevels, unsigned int numSubSamples, Dimension dimension);
	bool CreateUnorderedAccessView(ID3D11Device* device, DXGI_FORMAT format, unsigned int arraySize,
								   unsigned int mipLevels, Dimension dimension);

public:
	Texture() : resource(nullptr), shaderResourceView(nullptr) {}
	virtual ~Texture();

	Texture(const Texture&) = delete;
	const Texture& operator = (const Texture&) = delete;

private:
	ID3D11Resource* resource;
	ID3D11ShaderResourceView* shaderResourceView;
	std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews;
};