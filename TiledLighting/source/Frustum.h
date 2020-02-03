#pragma once

#include <array>
#include <DirectXMath.h>

class Frustum
{
public:
    Frustum();
	Frustum(DirectX::FXMMATRIX invViewProjectionMatrix);
	Frustum(DirectX::FXMVECTOR aabMin, DirectX::FXMVECTOR aabMax);
	bool XM_CALLCONV CollisionCheck(DirectX::FXMVECTOR position, float radius) const;
	bool XM_CALLCONV CollisionCheck(DirectX::FXMVECTOR boundMix, DirectX::FXMVECTOR boundMax) const;

private:
	std::array<DirectX::XMVECTOR, 6> planes;
};
