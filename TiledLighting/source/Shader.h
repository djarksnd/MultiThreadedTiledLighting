#pragma once

#include <string>
#include <vector>
#include <D3D11.h>

class Shader
{
public:
	enum class Type
	{
		Compute = 0,
		Geometry,
		Pixel,
		Vertex,

		NumShaderTypes
	};

	struct Macro
	{
		std::string name;
		std::string definition;
	};

	static std::vector<Macro> EmptyMacro;

public:
	virtual Type GetType() const = 0;
	virtual void Destroy() = 0;

protected:
	bool Compile(ID3DBlob** out, const std::wstring& file, const std::string& funtion, const std::vector<Macro>& macros = EmptyMacro);
	void OutputCompileErrorMessage(ID3DBlob* d3dMessage, const std::wstring& file);

protected:
	Shader() {}

public:
	virtual ~Shader() {}
};