#pragma once

#include <D3D11.h>

class BlendState
{
public:
	bool Create(ID3D11Device* device, const D3D11_BLEND_DESC& desc);
	void Destroy();
	operator ID3D11BlendState* () const { return blendState; }

public:
	BlendState() : blendState(nullptr) {}
	~BlendState() { Destroy(); }

private:
	ID3D11BlendState* blendState;
};