#pragma once

#include <D3D11.h>
#include <memory>
#include "Texture.h"

class DepthStencilBuffer : private Texture
{
public:
	enum class Format
	{
		Depth16 = 0,
		Depth24_Stencil8,
		Depth32,
		Depth32_Stencil8,
	};

public:
	bool Create(ID3D11Device* device, Format format, 
				unsigned int width, unsigned int height, unsigned int arraySize,
				unsigned int numSubSamples, Texture::Dimension dimension);
	void Destroy();
	void Clear(ID3D11DeviceContext* deviceContext, float depth, unsigned char stencil) const;

	operator ID3D11ShaderResourceView* () const {
		return Texture::operator ID3D11ShaderResourceView *();
	}
	ID3D11DepthStencilView* GetReadOnlyDepthStencilView() const {
		return readOnlyDepthStencilView;
	}
	operator ID3D11DepthStencilView* () const {
		return depthStencilView;
	}

	DepthStencilBuffer() : depthStencilView(nullptr), readOnlyDepthStencilView(nullptr) {}
	~DepthStencilBuffer();

	DepthStencilBuffer(const DepthStencilBuffer&) = delete;
	const DepthStencilBuffer& operator = (const DepthStencilBuffer&) = delete;

private:
	bool CreateDepthStencilViews(ID3D11Device* device, Format format, unsigned int arraySize, 
								 unsigned int numSubSamples, Texture::Dimension dimension);

private:
	ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilView* readOnlyDepthStencilView;
};