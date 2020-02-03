#include <assert.h>
#include "ComputeShader.h"



void ComputeShader::Destroy()
{
	if (shader)
	{
		shader->Release();
		shader = nullptr;
	}
}

bool ComputeShader::Create(ID3D11Device* device, const std::wstring& file,
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

	HRESULT hResult = device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
	blob->Release();

	if (FAILED(hResult))
	{
		assert(false && "faild to create computShader");
		return false;
	}

	return true;
}
