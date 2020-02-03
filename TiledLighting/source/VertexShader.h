#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
public:
	virtual Type GetType() const { return Shader::Type::Vertex; }
	virtual void Destroy();
	bool Create(ID3D11Device* device, const std::wstring& file, const std::string& funtion,
		const D3D11_INPUT_ELEMENT_DESC inputElenemts[], unsigned int numInputElenemts, const std::vector<Macro>& macros = Shader::EmptyMacro);
	operator ID3D11VertexShader* () const { return shader; }
	ID3D11InputLayout* GetInputLayout() const { return inputLayout; }

	VertexShader() : shader(nullptr), inputLayout(nullptr) {}
	~VertexShader() { Destroy(); }

private:
	VertexShader(const VertexShader&) = delete;
	const VertexShader& operator = (const VertexShader&) = delete;

private:
	ID3D11VertexShader* shader;
	ID3D11InputLayout* inputLayout;
};
