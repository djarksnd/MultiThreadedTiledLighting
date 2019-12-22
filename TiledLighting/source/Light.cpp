#include "Light.h"

DirectX::XMMATRIX ComputePointLightShadowMatrix(const PointLight& light, PointLight::ShadowFace face)
{
	static const DirectX::XMVECTORF32 look[6] =
	{
		{ 1.0f,  0.0f,  0.0f, 0.0f},
		{-1.0f,  0.0f,  0.0f, 0.0f},
		{ 0.0f,  1.0f,  0.0f, 0.0f},
		{ 0.0f, -1.0f,  0.0f, 0.0f},
		{ 0.0f,  0.0f,  1.0f, 0.0f},
		{ 0.0f,  0.0f, -1.0f, 0.0f}
	};

	static const DirectX::XMVECTORF32 up[6] =
	{
		{ 0.0f,  1.0f,  0.0f, 0.0f},
		{ 0.0f,  1.0f,  0.0f, 0.0f},
		{ 0.0f,  0.0f, -1.0f, 0.0f},
		{ 0.0f,  0.0f,  1.0f, 0.0f},
		{ 0.0f,  1.0f,  0.0f, 0.0f},
		{ 0.0f,  1.0f,  0.0f, 0.0f}
	};

	const DirectX::XMVECTOR lightPosition = DirectX::XMLoadFloat3(&light.position);
	const DirectX::XMMATRIX shadowViewMatrix = DirectX::XMMatrixLookAtLH(
		lightPosition, DirectX::XMVectorAdd(lightPosition,
		look[static_cast<size_t>(face)]),
		up[static_cast<size_t>(face)]);
	
	const DirectX::XMMATRIX shadowProjctionMatrix =
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 1.0f, light.radius, 0.1f);

	return DirectX::XMMatrixMultiply(shadowViewMatrix, shadowProjctionMatrix);
}

Frustum ComputePointLightShadowFrustum(const PointLight& light)
{
	DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&light.position);
	const DirectX::XMVECTOR min =
        DirectX::XMVectorSubtract(position, DirectX::XMVectorReplicate(light.radius));
	const DirectX::XMVECTOR max =
        DirectX::XMVectorAdd(position, DirectX::XMVectorReplicate(light.radius));

	return Frustum(min, max);
}

DirectX::XMMATRIX ComputeSpotLightShadowMatrix(const SpotLight& light)
{
	const DirectX::XMVECTOR lightPosition = DirectX::XMLoadFloat3(&light.position);
	const DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(lightPosition, DirectX::XMLoadFloat3(&light.direction));
	const DirectX::XMVECTOR up = (light.direction.y < 0.9f && light.direction.y > -0.9f) ?
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) : DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	const float fovY = acosf(light.cosHalfAngle) * 2.0f;

	return DirectX::XMMatrixMultiply(DirectX::XMMatrixLookAtLH(lightPosition, lookAt, up),
									 DirectX::XMMatrixPerspectiveFovLH(fovY, 1.0f, light.radius, 0.1f));
}
