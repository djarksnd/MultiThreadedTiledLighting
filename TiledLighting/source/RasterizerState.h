#pragma once

#include <D3D11.h>

class RasterizerState
{
public:
	bool Create(ID3D11Device* device, const D3D11_RASTERIZER_DESC& desc);
	bool Create(ID3D11Device* device, D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID,
		D3D11_CULL_MODE cullMode = D3D11_CULL_BACK, bool clockwise = false, bool multiSample = false,
		bool antialiasedLine = false, int depthBias = 0, float depthBiasClamp = 0.0f, 
		float slopeScaledDepthBias = 0.0f, bool depthClip = true, bool scissor = false);

	void Destroy();
	operator ID3D11RasterizerState* () const { return rasterizerState; }

	RasterizerState() : rasterizerState(nullptr) {}
	~RasterizerState() { Destroy(); }

private:
	ID3D11RasterizerState* rasterizerState;
};
