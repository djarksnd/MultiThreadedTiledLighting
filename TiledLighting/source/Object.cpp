#include <iostream>

#include "Object.h"
#include "Frustum.h"

bool Object::Create(ID3D11Device* device, ID3D11DeviceContext* ImmediateContext, const std::wstring& fileName)
{
	if (FAILED(mesh.Create(device, fileName.c_str())))
		return false;

	orgSubsetBounds.resize(mesh.GetNumMeshes());
	transformedSubsetBounds.resize(mesh.GetNumMeshes());

	for (unsigned int meshIndex = 0; meshIndex < mesh.GetNumMeshes(); ++meshIndex)
	{
		std::vector<DirectX::XMFLOAT3> vertices;
		std::vector<unsigned int> indices;

		//////////////////////////////////////////////////////////////////////////
		// read vertices
		//////////////////////////////////////////////////////////////////////////
		{
			ID3D11Buffer* vertexBuffer = mesh.GetVB11(meshIndex, 0);
			
			D3D11_BUFFER_DESC desc;
			vertexBuffer->GetDesc(&desc);
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			ID3D11Buffer* stagingBuffer = nullptr;
			device->CreateBuffer(&desc, nullptr, &stagingBuffer);
			ImmediateContext->CopyResource(stagingBuffer, vertexBuffer);

			D3D11_MAPPED_SUBRESOURCE mappedSubResource;
			ImmediateContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubResource);

			byte* data = static_cast<byte*>(mappedSubResource.pData);
			const unsigned int stride = mesh.GetVertexStride(meshIndex, 0);
			unsigned int readPos = 0;

			vertices.reserve(desc.ByteWidth / stride);
			while (readPos < desc.ByteWidth)
			{
				DirectX::XMFLOAT3* position = reinterpret_cast<DirectX::XMFLOAT3*>(data + readPos);
				vertices.push_back(*position);

				readPos += stride;
			}

			ImmediateContext->Unmap(stagingBuffer, 0);
			stagingBuffer->Release();
		}

		//////////////////////////////////////////////////////////////////////////
		// read indices
		//////////////////////////////////////////////////////////////////////////
		{
			ID3D11Buffer* indexBuffer = mesh.GetIB11(meshIndex);

			D3D11_BUFFER_DESC desc;
			indexBuffer->GetDesc(&desc);
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			ID3D11Buffer* stagingBuffer = nullptr;
			device->CreateBuffer(&desc, nullptr, &stagingBuffer);
			ImmediateContext->CopyResource(stagingBuffer, indexBuffer);

			D3D11_MAPPED_SUBRESOURCE mappedSubResource;
			ImmediateContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubResource);

			byte* data = static_cast<byte*>(mappedSubResource.pData);
			const unsigned int stride = (mesh.GetIndexType(meshIndex) == IT_16BIT) ? 2 : 4;
			unsigned int readPos = 0;

			indices.reserve(desc.ByteWidth / stride);
			while (readPos < desc.ByteWidth)
			{
				if (stride == 2)
					indices.push_back(*(reinterpret_cast<UINT16*>(data + readPos)));
				else
					indices.push_back(*(reinterpret_cast<UINT32*>(data + readPos)));
				
				readPos += stride;
			}

			ImmediateContext->Unmap(stagingBuffer, 0);
			stagingBuffer->Release();
		}

		//////////////////////////////////////////////////////////////////////////
		// calc subsets bounds
		//////////////////////////////////////////////////////////////////////////
		orgSubsetBounds[meshIndex].resize(mesh.GetNumSubsets(meshIndex));
		transformedSubsetBounds[meshIndex].resize(mesh.GetNumSubsets(meshIndex));

		for (unsigned int subsetIndex = 0; subsetIndex < mesh.GetNumSubsets(meshIndex); subsetIndex++)
		{
			const SDKMESH_SUBSET* subset = mesh.GetSubset(meshIndex, subsetIndex);
			const unsigned int IndexCount = static_cast<unsigned int>(subset->IndexCount);
			const unsigned int IndexStart = static_cast<unsigned int>(subset->IndexStart);
			const unsigned int VertexStart = static_cast<unsigned int>(subset->VertexStart);

			DirectX::XMVECTOR boundMin = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
			DirectX::XMVECTOR boundMax = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

			for (unsigned int i = IndexStart; i < IndexStart + IndexCount; ++i)
			{
				const DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices[indices[i] + VertexStart]);
				boundMin = DirectX::XMVectorMin(boundMin, vertex);
				boundMax = DirectX::XMVectorMax(boundMax, vertex);
			}

			DirectX::XMStoreFloat3(&orgSubsetBounds[meshIndex][subsetIndex].min, boundMin);
			DirectX::XMStoreFloat3(&orgSubsetBounds[meshIndex][subsetIndex].max, boundMax);
			DirectX::XMStoreFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].min, boundMin);
			DirectX::XMStoreFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].max, boundMax);
		}
	}
	
	 return true;
}

void Object::Update()
{
	for (unsigned int meshIndex = 0; meshIndex < mesh.GetNumMeshes(); ++meshIndex)
	{
		for (unsigned int subsetIndex = 0; subsetIndex < mesh.GetNumSubsets(meshIndex); subsetIndex++)
		{
			DirectX::XMFLOAT3 boundsVertices[8] = {
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].min.x,
					orgSubsetBounds[meshIndex][subsetIndex].min.y,
					orgSubsetBounds[meshIndex][subsetIndex].min.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].max.x,
					orgSubsetBounds[meshIndex][subsetIndex].min.y,
					orgSubsetBounds[meshIndex][subsetIndex].min.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].max.x,
					orgSubsetBounds[meshIndex][subsetIndex].min.y,
					orgSubsetBounds[meshIndex][subsetIndex].max.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].min.x,
					orgSubsetBounds[meshIndex][subsetIndex].min.y,
					orgSubsetBounds[meshIndex][subsetIndex].max.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].min.x,
					orgSubsetBounds[meshIndex][subsetIndex].max.y,
					orgSubsetBounds[meshIndex][subsetIndex].min.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].max.x,
					orgSubsetBounds[meshIndex][subsetIndex].max.y,
					orgSubsetBounds[meshIndex][subsetIndex].min.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].max.x,
					orgSubsetBounds[meshIndex][subsetIndex].max.y,
					orgSubsetBounds[meshIndex][subsetIndex].max.z),
				DirectX::XMFLOAT3(
					orgSubsetBounds[meshIndex][subsetIndex].min.x,
					orgSubsetBounds[meshIndex][subsetIndex].max.y,
					orgSubsetBounds[meshIndex][subsetIndex].max.z)
			};

			DirectX::XMVECTOR min = DirectX::XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
			DirectX::XMVECTOR max = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (auto& vertex : boundsVertices)
			{
				DirectX::XMVECTOR vert = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&vertex), tramsform);

				min = DirectX::XMVectorMin(min, vert);
				max = DirectX::XMVectorMax(max, vert);
			}

			DirectX::XMStoreFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].min, min);
			DirectX::XMStoreFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].max, max);
		}
	}
}

void Object::Render(ID3D11DeviceContext* deviceContext, const Frustum& frustum)
{
	if (0 < mesh.GetOutstandingBufferResources())
		return;

	for (unsigned int meshIndex = 0; meshIndex < mesh.GetNumMeshes(); ++meshIndex)
	{
		const SDKMESH_MESH* sdkMesh = mesh.GetMesh(meshIndex);

		unsigned int strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		unsigned int offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11Buffer* vertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

		if (sdkMesh->NumVertexBuffers > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT)
			return;

		for (byte vertexBufferIndex = 0; vertexBufferIndex < sdkMesh->NumVertexBuffers; ++vertexBufferIndex)
		{
			vertexBuffers[vertexBufferIndex] = mesh.GetVB11(meshIndex, vertexBufferIndex);
			strides[vertexBufferIndex] = mesh.GetVertexStride(meshIndex, vertexBufferIndex);
			offsets[vertexBufferIndex] = 0;
		}

		const DXGI_FORMAT indexBufferFormat = (mesh.GetIndexType(meshIndex) == IT_16BIT) ?
			DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

		deviceContext->IASetVertexBuffers(0, sdkMesh->NumVertexBuffers, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(mesh.GetIB11(meshIndex), indexBufferFormat, 0);

		for (unsigned int subsetIndex = 0; subsetIndex < mesh.GetNumSubsets(meshIndex); subsetIndex++)
		{
			{
				if (!frustum.Test(
					DirectX::XMLoadFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].min),
					DirectX::XMLoadFloat3(&transformedSubsetBounds[meshIndex][subsetIndex].max)))
					continue;
			}

			const SDKMESH_SUBSET* subset = mesh.GetSubset(meshIndex, subsetIndex);
			const D3D11_PRIMITIVE_TOPOLOGY PrimType =
				mesh.GetPrimitiveType11(static_cast<SDKMESH_PRIMITIVE_TYPE>(subset->PrimitiveType));

			deviceContext->IASetPrimitiveTopology(PrimType);

			const SDKMESH_MATERIAL* material = mesh.GetMaterial(subset->MaterialID);
			ID3D11ShaderResourceView* srvs[] = {
				!IsErrorResource(material->pDiffuseRV11) ? material->pDiffuseRV11 : nullptr,
				!IsErrorResource(material->pNormalRV11) ? material->pNormalRV11 : nullptr };

			deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

			unsigned int IndexCount = static_cast<unsigned int>(subset->IndexCount);
			unsigned int IndexStart = static_cast<unsigned int>(subset->IndexStart);
			unsigned int VertexStart = static_cast<unsigned int>(subset->VertexStart);

			deviceContext->DrawIndexed(IndexCount, IndexStart, VertexStart);
		}
	}
}

void Object::GetBoundMixMax(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax) const
{
	DirectX::XMVECTOR min = DirectX::XMVectorSubtract(mesh.GetMeshBBoxCenter(0), mesh.GetMeshBBoxExtents(0));
	DirectX::XMVECTOR max = DirectX::XMVectorAdd(mesh.GetMeshBBoxCenter(0), mesh.GetMeshBBoxExtents(0));

	for (unsigned int i = 1; i < mesh.GetNumMeshes(); i++)
	{
		min = DirectX::XMVectorMin(min, DirectX::XMVectorSubtract(mesh.GetMeshBBoxCenter(i), mesh.GetMeshBBoxExtents(i)));
		max = DirectX::XMVectorMax(max, DirectX::XMVectorAdd(mesh.GetMeshBBoxCenter(i), mesh.GetMeshBBoxExtents(i)));
	}

	DirectX::XMStoreFloat3(&outMin, min);
	DirectX::XMStoreFloat3(&outMax, max);
}
