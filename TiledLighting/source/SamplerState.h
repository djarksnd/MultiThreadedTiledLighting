#pragma once

#include <D3D11.h>

class SamplerState
{
public:
	bool Create(ID3D11Device* device, const D3D11_SAMPLER_DESC& desc);
	void Destroy();
	operator ID3D11SamplerState* () const { return samplerState; }

public:
	SamplerState() : samplerState(nullptr) {}
	~SamplerState() { Destroy(); }

private:
	ID3D11SamplerState* samplerState;
};