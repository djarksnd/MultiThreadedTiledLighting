#include <assert.h>
#include "VertexShader.h"



void VertexShader::Destroy()
{
    if (inputLayout)
    {
        inputLayout->Release();
        inputLayout = nullptr;
    }

    if (shader)
    {
        shader->Release();
        shader = nullptr;
    }
}



bool VertexShader::Create(
    ID3D11Device* device, const std::wstring& file,
    const std::string& funtion, const D3D11_INPUT_ELEMENT_DESC inputElenemts[],
    unsigned int numInputElenemts, const std::vector<Macro>& macros)
{
    assert(!file.empty());
    assert(!funtion.empty());
    assert(inputElenemts);
    assert(numInputElenemts);

    if (file.empty() || funtion.empty() || nullptr == inputElenemts || 0 == numInputElenemts)
    {
        return false;
    }

    ID3DBlob*blob = nullptr;
    if (!Compile(&blob, file, funtion, macros))
    {
        return false;
    }

    HRESULT hResult = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
    if (FAILED(hResult))
    {
        blob->Release();
        assert(false && "failed to CreateVertexShader.");
        return false;
    }

    hResult = device->CreateInputLayout(inputElenemts, numInputElenemts, blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout);
    blob->Release();

    if (FAILED(hResult))
    {
        assert(false && "failed to CreateInputLayout.");
        return false;
    }

    return true;
}
