#pragma once

#include <D3D11.h>
#include "ResourceCPUAccessFlags.h"

class VertexBuffer
{
private:
	ID3D11Buffer* buffer;
	unsigned int stride;
	unsigned int offset;

public:
	bool Create(ID3D11Device* device, unsigned int size, bool streamOut,
				const ResourceCPUAccessFlags& cpuAccessFlags, void* data);
	void Destroy();
	unsigned int GetStride() const { return stride; }
	unsigned int GetOffser() const { return offset; }
	operator ID3D11Buffer* () const { return buffer; }

public:
	VertexBuffer() : buffer(nullptr), stride(0), offset(0) {}
	~VertexBuffer() { Destroy(); }
};