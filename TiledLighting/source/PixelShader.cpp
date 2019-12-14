#include <assert.h>
#include "PixelShader.h"

void
PixelShader::Destroy()
{
	if (shader)
	{
		shader->Release();
		shader = nullptr;
	}
}

bool
PixelShader::Create(ID3D11Device* device, const std::wstring& file,
					const std::string& funtion, const std::vector<Macro>& macros)
{
	assert(!file.empty());
	assert(!funtion.empty());

	if (file.empty() || funtion.empty())
	{
		return false;
	}

	ID3DBlob* blob = nullptr;
	if (!Compile(&blob, file, funtion, macros))
	{
		return false;
	}

	HRESULT hResult = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
	blob->Release();

	if (FAILED(hResult))
	{
		assert(false && "PixelShader 생성 실패.");
		return false;
	}

	return true;
}