#pragma once

#include <d3d11.h>
#include "VertexBuffer.h"
#include "VertexShader.h"

class PixelShader;

class ScreenAlignQuad
{
public:
	bool Create(ID3D11Device* device);
	void Render(ID3D11DeviceContext* deviceContext) const;

private:
	VertexBuffer vertexBuffer;
	VertexShader vertexShader;
};