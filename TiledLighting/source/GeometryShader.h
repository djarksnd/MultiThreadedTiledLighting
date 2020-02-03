#pragma once

#include "Shader.h"

class GeometryShader : public Shader
{
public:
    virtual Type GetType() const { return Shader::Type::Geometry; }
    virtual void Destroy();
    bool Create(ID3D11Device* device, const std::wstring& file, const std::string& funtion,
        const std::vector<Macro>& macros = Shader::EmptyMacro);

    operator ID3D11GeometryShader* () const { return shader; }

    GeometryShader() : shader(nullptr) {}
    ~GeometryShader() { Destroy(); }

private:
    GeometryShader(const GeometryShader&) = delete;
    const GeometryShader& operator = (const GeometryShader&) = delete;

private:
    ID3D11GeometryShader* shader;
};
