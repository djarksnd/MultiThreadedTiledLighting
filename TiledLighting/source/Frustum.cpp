#include "Frustum.h"

Frustum::Frustum(DirectX::FXMMATRIX invViewProjectionMatrix)
{
	const DirectX::XMVECTOR vertices[8] =
	{
		// front
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1.0f,  1.0f, 1.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f,  1.0f, 1.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f, -1.0f, 1.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1.0f, -1.0f, 1.0f, 0.0f), invViewProjectionMatrix),

		// back
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1.0f,  1.0f, 0.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f,  1.0f, 0.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f, -1.0f, 0.0f, 0.0f), invViewProjectionMatrix),
		DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1.0f, -1.0f, 0.0f, 0.0f), invViewProjectionMatrix)
	};

	planes[0] = DirectX::XMPlaneFromPoints(vertices[1], vertices[0], vertices[2]); // front
	planes[1] = DirectX::XMPlaneFromPoints(vertices[4], vertices[5], vertices[7]); // back
	planes[2] = DirectX::XMPlaneFromPoints(vertices[0], vertices[4], vertices[3]); // left
	planes[3] = DirectX::XMPlaneFromPoints(vertices[1], vertices[5], vertices[0]); // top
	planes[4] = DirectX::XMPlaneFromPoints(vertices[5], vertices[1], vertices[6]); // right
	planes[5] = DirectX::XMPlaneFromPoints(vertices[3], vertices[7], vertices[2]); // bottom
}

Frustum::Frustum(DirectX::FXMVECTOR aabMin, DirectX::FXMVECTOR aabMax)
{
	planes[0] = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, -DirectX::XMVectorGetZ(aabMin)); // front
	planes[1] = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, DirectX::XMVectorGetZ(aabMax)); // back
	planes[2] = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, -DirectX::XMVectorGetX(aabMin)); // left
	planes[3] = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, DirectX::XMVectorGetY(aabMax)); // top
	planes[4] = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, DirectX::XMVectorGetX(aabMax)); // right
	planes[5] = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, -DirectX::XMVectorGetY(aabMin)); // bottom
}

bool Frustum::Test(DirectX::FXMVECTOR position, float radius) const
{
	for (auto& plane : planes)
	{
		const float result = DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(plane, position));
		if (result + radius < 0.0f)
			return false;
	}

	return true;
}

bool Frustum::Test(DirectX::FXMVECTOR boundMix, DirectX::FXMVECTOR boundMax) const
{
	for (auto& plane : planes)
	{
		DirectX::XMFLOAT3 point;
		if (DirectX::XMVectorGetX(plane) > 0.0f)
			point.x = DirectX::XMVectorGetX(boundMax);
		else
			point.x = DirectX::XMVectorGetX(boundMix);
		if (DirectX::XMVectorGetY(plane) > 0.0f)
			point.y = DirectX::XMVectorGetY(boundMax);
		else
			point.y = DirectX::XMVectorGetY(boundMix);
		if (DirectX::XMVectorGetZ(plane) > 0.0f)
			point.z = DirectX::XMVectorGetZ(boundMax);
		else
			point.z = DirectX::XMVectorGetZ(boundMix);

		const float result = DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(plane, DirectX::XMLoadFloat3(&point)));
		if (result < 0.0f)
			return false;
	}

	return true;
}