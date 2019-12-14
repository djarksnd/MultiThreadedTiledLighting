#include <assert.h>
#include "ConstantBuffer.h"

void
ConstantBuffer::Destroy()
{
	if (buffer)
	{
		buffer->Release();
		buffer = nullptr;
		size = 0;
	}
}

bool
ConstantBuffer::Update(ID3D11DeviceContext* deviceContext, const void* const data) const
{
	assert(data);
	if (nullptr == data)
	{
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hResult = deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hResult))
	{
		assert(false && "Failed to Map");
		return false;
	}

	memcpy_s(mappedResource.pData, size, data, size);

	deviceContext->Unmap(buffer, 0);

	return true;
}

bool
ConstantBuffer::Create(ID3D11Device* device, unsigned int argSize)
{
	assert(argSize);
	if (0 == argSize)
	{
		return false;
	}

	if (argSize % SizeOffset)
	{
		argSize = (argSize / SizeOffset) * SizeOffset + SizeOffset;
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = argSize;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hResult = device->CreateBuffer(&bufferDesc, nullptr, &buffer);
	if (FAILED(hResult))
	{
		assert(false && "Failed to CreateBuffer.");
		return false;
	}

	size = argSize;

	return true;
}