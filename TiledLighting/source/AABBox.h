#pragma once

#include <DirectXMath.h>

class AABBox
{
public:
    AABBox() : boundMin(FLT_MAX, FLT_MAX, FLT_MAX), boundMax(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
    AABBox(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) : boundMin(min), boundMax(max)  {}

    AABBox& SetMin(DirectX::XMFLOAT3& min) {
        boundMin = min;
        return *this;
    }
    AABBox& SetMax(DirectX::XMFLOAT3& max) {
        boundMax = max;
        return *this;
    }
    const DirectX::XMFLOAT3& GetMin() const {
        return boundMin;
    }
    const DirectX::XMFLOAT3& GetMax() const {
        return boundMax;
    }

    AABBox& XM_CALLCONV AddPoint(DirectX::FXMVECTOR point);
    AABBox& XM_CALLCONV AddMinMaxPoint(DirectX::FXMVECTOR min, DirectX::FXMVECTOR max);
    AABBox& XM_CALLCONV Transform(DirectX::FXMMATRIX transform);

    const AABBox operator + (const AABBox& other) const;
    AABBox& operator += (const AABBox& other);

private:
    DirectX::XMFLOAT3 boundMin;
    DirectX::XMFLOAT3 boundMax;
};
