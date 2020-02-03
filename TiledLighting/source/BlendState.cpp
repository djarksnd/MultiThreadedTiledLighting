#include <assert.h>
#include "BlendState.h"

bool BlendState::Create(ID3D11Device* device, const D3D11_BLEND_DESC& desc)
{
	HRESULT result = device->CreateBlendState(&desc, &blendState);
	if (FAILED(result))
	{
		assert(false && "failed to create BlendState");
		return false;
	}

	return true;
}

void BlendState::Destroy()
{
	if (blendState)
	{
		blendState->Release();
		blendState = nullptr;
	}
}
