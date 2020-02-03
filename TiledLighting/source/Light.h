#pragma once

#include <DirectXMath.h>
#include "Frustum.h"

// 16bytes align for cbuffer binding. 
#pragma pack(push, 16)
struct PointLight
{
	DirectX::XMFLOAT3 position;
	float radius;
	DirectX::XMFLOAT3 color;
	float falloff;

	enum class ShadowFace
	{
		right = 0,
		left,
		top,
		bottom,
		front,
		back,
	};
};

struct SpotLight
{
	DirectX::XMFLOAT3 position;
	float radius;
	DirectX::XMFLOAT3 color;
	float falloff;
	DirectX::XMFLOAT3 direction;
	float cosHalfAngle;
};
#pragma pack(pop)

DirectX::XMMATRIX ComputePointLightShadowMatrix(const PointLight& light, PointLight::ShadowFace face);
DirectX::XMMATRIX ComputeSpotLightShadowMatrix(const SpotLight& light);
Frustum ComputePointLightShadowFrustum(const PointLight& light);
