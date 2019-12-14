#pragma once

#include <D3D11.h>
#include <vector>
#include <memory>

#include "Texture.h"

class RenderTarget : private Texture
{
public:
	bool Create(ID3D11Device* device, DXGI_FORMAT format, 
				const ResourceBindFlags& bindFlags,
				unsigned int width, unsigned int height,
				unsigned int mipLevels, unsigned int numSubSamples);

	virtual void Destroy();
	void Clear(ID3D11DeviceContext* deviceContext, const float color[4]) const;

	operator ID3D11ShaderResourceView* () const {
		return Texture::operator ID3D11ShaderResourceView *();
	}
	operator ID3D11RenderTargetView* () const {
		return renderTargetViews[0];
	}
	ID3D11RenderTargetView* operator [] (unsigned int mipLevel) const {
		return renderTargetViews[mipLevel];
	}

	RenderTarget() {}
	~RenderTarget();

	RenderTarget(const RenderTarget&) = delete;
	const RenderTarget& operator = (const RenderTarget&) = delete;

private:
	bool CreateRenderTargetViews(ID3D11Device* device, DXGI_FORMAT format, unsigned int mipLevels, unsigned int numSubSamples);

private:
	std::vector<ID3D11RenderTargetView*> renderTargetViews;
};