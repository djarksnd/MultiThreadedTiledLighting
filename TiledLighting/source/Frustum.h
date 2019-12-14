#pragma once

#include <array>
#include <DirectXMath.h>

class Frustum
{
public:
	Frustum() {}
	Frustum(DirectX::FXMMATRIX invViewProjectionMatrix);
	Frustum(DirectX::FXMVECTOR aabMin, DirectX::FXMVECTOR aabMax);
	bool Test(DirectX::FXMVECTOR position, float radius) const;
	bool Test(DirectX::FXMVECTOR boundMix, DirectX::FXMVECTOR boundMax) const;

private:
	std::array<DirectX::XMVECTOR, 6> planes;
};