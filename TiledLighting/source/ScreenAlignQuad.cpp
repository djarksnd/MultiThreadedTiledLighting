#include "ScreenAlignQuad.h"

bool ScreenAlignQuad::Create(ID3D11Device* device)
{
	float vertices[] = {
		-1.0f,  1.0f, 
		 1.0f,  1.0f, 
		-1.0f, -1.0f, 
		 1.0f, -1.0f };

	if (!vertexBuffer.Create(device, sizeof(vertices), false, 0, vertices))
		return false;

	const D3D11_INPUT_ELEMENT_DESC inputElements[] = { 
		"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (!vertexShader.Create(device, L"..\\media\\shader\\ScreenAlignQuadVertexShader.hlsl", "main", 
		inputElements, ARRAYSIZE(inputElements)))
		return false;

	return true;
}

void ScreenAlignQuad::Render(ID3D11DeviceContext* deviceContext) const
{
	deviceContext->IASetInputLayout(vertexShader.GetInputLayout());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D11Buffer* buffer = vertexBuffer;
	unsigned int stride = sizeof(float) * 2;
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

	deviceContext->VSSetShader(vertexShader, nullptr, 0);

	deviceContext->Draw(4, 0);

	buffer = nullptr;
	deviceContext->IASetInputLayout(nullptr);
	deviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	deviceContext->VSSetShader(nullptr, nullptr, 0);
}