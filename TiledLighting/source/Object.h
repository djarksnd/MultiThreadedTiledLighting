#pragma once

#include <string>
#include <DirectXMath.h>
#include <vector>

#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Optional\\SDKmesh.h"

class Frustum;

class Object
{
public:
	struct Bound
	{
		DirectX::XMFLOAT3 min;
		DirectX::XMFLOAT3 max;
	};

public:
	Object() : tramsform(DirectX::XMMatrixIdentity()) {}
	~Object() {
		mesh.Destroy();
	}

	bool Create(ID3D11Device* device, ID3D11DeviceContext* ImmediateContext, const std::wstring& fileName);
	void Update();
	void Render(ID3D11DeviceContext* deviceContext, const Frustum& frustum);

	void GetBoundMixMax(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax) const;
	void SetTransform(DirectX::FXMMATRIX argTransform) {
		tramsform = argTransform;
	}
	const DirectX::FXMMATRIX GetTransform() {
		return tramsform;
	}

	const CDXUTSDKMesh& GetMesh() const {
		return mesh;
	}

	Object(const Object&) = delete;
	const Object& operator = (const Object&) = delete;

private:
	CDXUTSDKMesh mesh;
	DirectX::XMMATRIX tramsform;
	Bound bound;
	std::vector<std::vector<Bound>> orgSubsetBounds;
	std::vector<std::vector<Bound>> transformedSubsetBounds;
};