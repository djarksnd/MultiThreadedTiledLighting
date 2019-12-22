#include "AABBox.h"

void XM_CALLCONV AABBox::AddPoint(DirectX::FXMVECTOR point)
{
    DirectX::XMStoreFloat3(&boundMin, DirectX::XMVectorMin(DirectX::XMLoadFloat3(&boundMin), point));
    DirectX::XMStoreFloat3(&boundMax, DirectX::XMVectorMax(DirectX::XMLoadFloat3(&boundMax), point));
}

void XM_CALLCONV AABBox::AddMinMaxPoint(DirectX::FXMVECTOR min, DirectX::FXMVECTOR max)
{
    DirectX::XMStoreFloat3(&boundMin, DirectX::XMVectorMin(DirectX::XMLoadFloat3(&boundMin), min));
    DirectX::XMStoreFloat3(&boundMax, DirectX::XMVectorMax(DirectX::XMLoadFloat3(&boundMax), max));
}

void XM_CALLCONV AABBox::Transform(DirectX::FXMMATRIX transform)
{
    const DirectX::XMVECTOR vertices[8] =
    {
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMin.x, boundMin.y, boundMin.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMax.x, boundMin.y, boundMin.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMax.x, boundMin.y, boundMax.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMin.x, boundMin.y, boundMax.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMin.x, boundMax.y, boundMin.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMax.x, boundMax.y, boundMin.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMax.x, boundMax.y, boundMax.z, 0.0f), transform),
        DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(boundMin.x, boundMax.y, boundMax.z, 0.0f), transform)
    };

    DirectX::XMVECTOR mxMin = DirectX::XMVectorReplicate(FLT_MAX);
    DirectX::XMVECTOR mxMax = DirectX::XMVectorReplicate(-FLT_MAX);

    for (auto& vertex : vertices)
    {
        mxMin = DirectX::XMVectorMin(mxMin, vertex);
        mxMax = DirectX::XMVectorMax(mxMax, vertex);
    }

    DirectX::XMStoreFloat3(&boundMin, mxMin);
    DirectX::XMStoreFloat3(&boundMax, mxMax);
}


const AABBox AABBox::operator + (const AABBox& other)
{
    DirectX::XMFLOAT3 AABBoxMin;
    DirectX::XMFLOAT3 AABBoxMax;

    DirectX::XMStoreFloat3(&AABBoxMin, DirectX::XMVectorMin(DirectX::XMLoadFloat3(&boundMin), DirectX::XMLoadFloat3(&other.boundMin)));
    DirectX::XMStoreFloat3(&AABBoxMax, DirectX::XMVectorMax(DirectX::XMLoadFloat3(&boundMax), DirectX::XMLoadFloat3(&other.boundMax)));

    return AABBox(AABBoxMin, AABBoxMax);
}

const AABBox& AABBox::operator += (const AABBox& other)
{
    DirectX::XMStoreFloat3(&boundMin, DirectX::XMVectorMin(DirectX::XMLoadFloat3(&boundMin), DirectX::XMLoadFloat3(&other.boundMin)));
    DirectX::XMStoreFloat3(&boundMax, DirectX::XMVectorMax(DirectX::XMLoadFloat3(&boundMax), DirectX::XMLoadFloat3(&other.boundMax)));

    return *this;
}

