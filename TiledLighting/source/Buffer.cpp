#include <assert.h>
#include "Buffer.h"

bool
Buffer::Create(ID3D11Device* device, DXGI_FORMAT format,
			   unsigned int argBufferSize, unsigned int elementSize, 
			   const ResourceBindFlags& bindFlags,
			   const ResourceCPUAccessFlags& cpuAccessFlags, 
			   const void* initData)
{
	assert(argBufferSize);
	if (0 == argBufferSize)
		return false;

	const bool isStructured = format == DXGI_FORMAT_UNKNOWN;

	D3D11_BUFFER_DESC buffDesc;
	buffDesc.ByteWidth = argBufferSize;
	buffDesc.BindFlags = bindFlags.ShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0;
	buffDesc.BindFlags |= bindFlags.UnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0;
	buffDesc.CPUAccessFlags = cpuAccessFlags.read ? D3D11_CPU_ACCESS_READ : 0;
	buffDesc.CPUAccessFlags |= cpuAccessFlags.write ? D3D11_CPU_ACCESS_WRITE : 0;
	buffDesc.MiscFlags = isStructured ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0;
	buffDesc.StructureByteStride = isStructured ? elementSize : 0;

	// Usage.
	if (cpuAccessFlags.read)
		buffDesc.Usage = D3D11_USAGE_STAGING;
	else if (cpuAccessFlags.write)
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
	else // CPUAccessFlag::None
	{
		if (bindFlags.UnorderedAccess)
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
		else
			buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&buffDesc, initData ? &data : nullptr, &buff);
	if (FAILED(result))
	{
		assert(false && "failed to CreateBuffer");
		return false;
	}

	bufferSize = argBufferSize;

	if (bindFlags.ShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.ElementOffset = 0;
		srvDesc.Buffer.ElementWidth = bufferSize / elementSize;

		result = device->CreateShaderResourceView(buff, &srvDesc, &srv);
		if (FAILED(result))
		{
			assert(false && "failed to create ShaderResourceView");
			return false;
		}
	}

	if (bindFlags.UnorderedAccess)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = bufferSize / elementSize;
		uavDesc.Buffer.Flags = 0;

		result = device->CreateUnorderedAccessView(buff, &uavDesc, &uav);
		if (FAILED(result))
		{
			assert(false && "failed to create UnorderedAccessView");
			return false;
		}
	}

	return true;
}


void
Buffer::Destroy()
{
	if (buff)
	{
		buff->Release();
		buff = nullptr;
		bufferSize = 0;
	}

	if (srv)
	{
		srv->Release();
		srv = nullptr;
	}

	if (uav)
	{
		uav->Release();
		uav = nullptr;
	}
}

bool
Buffer::Update(ID3D11DeviceContext* deviceContext, const void* const data, unsigned int updateBytes) const
{
	assert(data);
	if (nullptr == data)
	{
		return false;
	}

	if (updateBytes == 0)
		updateBytes = bufferSize;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hResult = deviceContext->Map(buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hResult))
	{
		assert(false && "failed to Map");
		return false;
	}

	memcpy_s(mappedResource.pData, updateBytes, data, updateBytes);

	deviceContext->Unmap(buff, 0);

	return true;
}
