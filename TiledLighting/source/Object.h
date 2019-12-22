#pragma once

#include <string>
#include <DirectXMath.h>
#include <vector>

#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Optional\\SDKmesh.h"

#include "AABBox.h"

class Frustum;

class Object
{
public:
	Object() : transform(DirectX::XMMatrixIdentity()) {}
	~Object() { mesh.Destroy(); }

	bool Create(ID3D11Device* device, ID3D11DeviceContext* ImmediateContext, const std::wstring& fileName);
	void Render(ID3D11DeviceContext* deviceContext, const Frustum& frustum);

	void XM_CALLCONV SetTransform(DirectX::FXMMATRIX argTransform) {
		transform = argTransform;
	}
	const DirectX::FXMMATRIX GetTransform() {
		return transform;
	}
	const CDXUTSDKMesh& GetMesh() const {
		return mesh;
	}
    const AABBox& GetBound() const {
        return bound;
    }

	Object(const Object&) = delete;
	const Object& operator = (const Object&) = delete;

private:
	CDXUTSDKMesh mesh;
	DirectX::XMMATRIX transform;
    AABBox bound;
	std::vector<std::vector<AABBox>> subsetBounds;
};
