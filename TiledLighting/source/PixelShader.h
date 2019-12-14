#pragma once

#include "Shader.h"

class PixelShader : public Shader
{
public:
	virtual Type GetType() const { return Shader::Type::Pixel; }
	virtual void Destroy();
	bool Create(ID3D11Device* device, const std::wstring& file, const std::string& funtion, const std::vector<Macro>& macros = Shader::EmptyMacro);
	operator ID3D11PixelShader* () const { return shader; }

public:
	PixelShader() : shader(nullptr) {}
	~PixelShader() { Destroy(); }

private:
	PixelShader(const PixelShader&) = delete;
	const PixelShader& operator = (const PixelShader&) = delete;

private:
	ID3D11PixelShader* shader;
};