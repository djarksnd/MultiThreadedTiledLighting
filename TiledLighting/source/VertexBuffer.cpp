#include <assert.h>
#include "VertexBuffer.h"

bool
VertexBuffer::Create(ID3D11Device* device, unsigned int size, bool streamOut,
					 const ResourceCPUAccessFlags& cpuAccessFlags, void* data)
{
	D3D11_BUFFER_DESC bfDesc;
	bfDesc.ByteWidth = size;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (streamOut)
		bfDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	
	if (cpuAccessFlags.write)
	{
		bfDesc.Usage = D3D11_USAGE_DYNAMIC;
		bfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		if (cpuAccessFlags.read)
			bfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		else
			bfDesc.CPUAccessFlags = 0;

		if (streamOut)
			bfDesc.Usage = D3D11_USAGE_DEFAULT;
		else
			bfDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}

	D3D11_SUBRESOURCE_DATA srData;
	srData.pSysMem = data;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	HRESULT hResult = device->CreateBuffer(&bfDesc, data ? &srData : nullptr, &buffer);
	if (FAILED(hResult))
	{
		assert(false && "failed to CreateBuffer.");
		return false;
	}

	return true;
}

void
VertexBuffer::Destroy()
{
	if (buffer)
	{
		buffer->Release();
		buffer = nullptr;

		stride = 0;
		offset = 0;
	}
}