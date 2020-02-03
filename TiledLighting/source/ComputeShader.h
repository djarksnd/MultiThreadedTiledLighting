#pragma once

#include "Shader.h"

class ComputeShader : public Shader
{
public:
	virtual Type GetType() const {
		return Shader::Type::Compute;
	}
	virtual void Destroy();
	bool Create(ID3D11Device* device, const std::wstring& file, const std::string& funtion,
        const std::vector<Macro>& macros = Shader::EmptyMacro);

    operator ID3D11ComputeShader* () const { return shader; }

	ComputeShader() : shader(nullptr) {}
	virtual ~ComputeShader() { Destroy(); }

private:
	ComputeShader(const ComputeShader&) = delete;
	const ComputeShader& operator = (const ComputeShader&) = delete;

private:
	ID3D11ComputeShader* shader;
};
