#include <assert.h>
#include "DepthStencilState.h"


bool
DepthStencilState::Create(ID3D11Device* device, const D3D11_DEPTH_STENCIL_DESC& desc)
{
	HRESULT result = device->CreateDepthStencilState(&desc, &depthStencilState);
	if (FAILED(result))
	{
		assert(false && "failed to CreateDepthStencilState");
		return false;
	}

	return true;
}



bool
DepthStencilState::Create(ID3D11Device* device, bool depthEnable, bool depthWrite,
						  D3D11_COMPARISON_FUNC depthFunc, bool stencilEnable, unsigned char stencilReadMask,
						  unsigned char stencilWriteMask, D3D11_STENCIL_OP stencilFailOp, D3D11_STENCIL_OP stencilDepthFailOp,
						  D3D11_STENCIL_OP stencilPassOp, D3D11_COMPARISON_FUNC stencilFunc)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = depthEnable;
	desc.DepthWriteMask = depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = depthFunc;
	desc.StencilEnable = stencilEnable;
	desc.StencilReadMask = stencilReadMask;
	desc.StencilWriteMask = stencilWriteMask;
	desc.FrontFace.StencilFailOp = stencilFailOp;
	desc.FrontFace.StencilDepthFailOp = stencilDepthFailOp;
	desc.FrontFace.StencilPassOp = stencilPassOp;
	desc.FrontFace.StencilFunc = stencilFunc;
	desc.BackFace = desc.FrontFace;

	return Create(device, desc);
}



void
DepthStencilState::Destroy()
{
	if (depthStencilState)
	{
		depthStencilState->Release();
		depthStencilState = nullptr;
	}
}
