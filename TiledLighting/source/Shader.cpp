#include <d3dcompiler.h>
#include <fstream>
#include "Shader.h"

std::vector<Shader::Macro> Shader::EmptyMacro;

bool
Shader::Compile(ID3DBlob** out, const std::wstring& file, const std::string& funtion, const std::vector<Macro>& macros)
{
	ID3DBlob* pErrMsg = nullptr;

	std::string profile;
	switch (GetType())
	{
	case Type::Vertex:
		profile = "vs_5_0";
		break;
	case Type::Geometry:
		profile = "gs_5_0";
		break;
	case Type::Pixel:
		profile = "ps_5_0";
		break;
	case Type::Compute:
		profile = "cs_5_0";
		break;
	default:
		return false;
	}

#ifdef DEBUG
	const unsigned int uFlags1 = D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS;
#else // DEBUG
	const unsigned int uFlags1 = D3DCOMPILE_ENABLE_STRICTNESS;
#endif // DEBUG
	const unsigned int uFlags2 = 0;

	std::vector<D3D_SHADER_MACRO> defines;
	for (auto& macro : macros)
	{
		defines.push_back({ macro.name.c_str(), macro.definition.c_str() });
	}
	defines.push_back({ nullptr, nullptr });

	HRESULT hResult = D3DCompileFromFile(file.c_str(), &defines[0], D3D_COMPILE_STANDARD_FILE_INCLUDE, funtion.c_str(), profile.c_str(),
										 uFlags1, uFlags2, out, &pErrMsg);
	if (FAILED(hResult))
	{
		if (pErrMsg)
		{
			OutputCompileErrorMessage(pErrMsg, file);
			pErrMsg->Release();
		}
		else
		{
			MessageBox(nullptr, file.c_str(), L"missing shader file", MB_OK);
		}

		return false;
	}

	return true;
}



void
Shader::OutputCompileErrorMessage(ID3DBlob* d3dMessage, const std::wstring& file)
{
	char* pCompileErrors;
	unsigned int uBufferSize;
	std::wofstream wofstream;

	pCompileErrors = (char*)(d3dMessage->GetBufferPointer());

	uBufferSize = (unsigned int)d3dMessage->GetBufferSize();

	wofstream.open(L"shader-error.txt");

	for (unsigned int i = 0; i < uBufferSize; ++i)
	{
		wofstream << pCompileErrors[i];
	}

	wofstream.close();

	MessageBox(nullptr, L"Error compiling shader. Check shader-error.txt for message.", file.c_str(), MB_OK);
}
