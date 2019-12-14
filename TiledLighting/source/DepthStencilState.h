#pragma once

#include <D3D11.h>

class DepthStencilState
{
public:
	bool Create(ID3D11Device* device, const D3D11_DEPTH_STENCIL_DESC& desc);
	
	
	bool Create(ID3D11Device* device, bool depthEnable = true, bool depthWrite = true,
		D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_LESS, bool stencilEnable = false,
		unsigned char stencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK, unsigned char stencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP stencilFailOp = D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP stencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP stencilPassOp = D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_FUNC stencilFunc = D3D11_COMPARISON_ALWAYS);

	void Destroy();
	operator ID3D11DepthStencilState* () const { return depthStencilState; }

public:
	DepthStencilState() : depthStencilState(nullptr) {}
	~DepthStencilState() { Destroy(); }

private:
	ID3D11DepthStencilState* depthStencilState;
};