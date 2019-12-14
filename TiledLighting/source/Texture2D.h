#pragma once

#include <d3d11.h>
#include <DXGIFormat.h>
#include <vector>

#include "ResourceBindFlags.h"
#include "ResourceCPUAccessFlags.h"

class Texture2D
{
public:
	bool Create(ID3D11Device* device, DXGI_FORMAT format,
				unsigned int width, unsigned int height,
				unsigned int mipLevels, unsigned int numSubSamples,
				const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags);

	void Destroy();

	operator ID3D11Texture2D* () const {
		return texture;
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
	bool CreateTrxture(ID3D11Device* device, DXGI_FORMAT format,
					   unsigned int width, unsigned int height,
					   unsigned int mipLevels, unsigned int numSubSamples,
					   const ResourceBindFlags& bindFlags, const ResourceCPUAccessFlags& cpuAccessFlags);

	bool CreateShaderResourceView(ID3D11Device* device, DXGI_FORMAT format, unsigned int mipLevels, unsigned int numSubSamples);
	bool CreateUnorderedAccessView(ID3D11Device* device, DXGI_FORMAT format, unsigned int mipLevels);

public:
	Texture2D() : texture(nullptr), shaderResourceView(nullptr) {}
	~Texture2D() {
		Destroy();
	}

	Texture2D(const Texture2D&) = delete;
	const Texture2D& operator = (const Texture2D&) = delete;

private:
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* shaderResourceView;
	std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews;
};