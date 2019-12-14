#pragma once

#include <DirectXMath.h>

struct ViewInfo
{
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMMATRIX viewProjectionMatrix;
	DirectX::XMMATRIX invViewMatrix;
	DirectX::XMMATRIX invProjectionMatrix;
	DirectX::XMMATRIX invViewProjectionMatrix;
	DirectX::XMFLOAT3 viewOrigin;
	unsigned int viewWidth;
	unsigned int viewHeight;
};