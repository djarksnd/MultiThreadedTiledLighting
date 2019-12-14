#include <assert.h>
#include "SamplerState.h"


bool
SamplerState::Create(ID3D11Device* device, const D3D11_SAMPLER_DESC& desc)
{
	HRESULT result = device->CreateSamplerState(&desc, &samplerState);
	if (FAILED(result))
	{
		assert(false && "Failed to create SamplerState");
		return false;
	}

	return true;
}



void
SamplerState::Destroy()
{
	if (samplerState)
	{
		samplerState->Release();
		samplerState = nullptr;
	}
}