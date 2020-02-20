#pragma once

#include <D3D11.h>
#include "AccessFlags.h"

class VertexBuffer
{
public:
	bool Create(ID3D11Device* device, unsigned int size, bool streamOut,
				const AccessFlags& cpuAccessFlags, void* data);
	void Destroy();
	unsigned int GetStride() const { return stride; }
	unsigned int GetOffser() const { return offset; }
	operator ID3D11Buffer* () const { return buffer; }

	VertexBuffer() : buffer(nullptr), stride(0), offset(0) {}
	~VertexBuffer() { Destroy(); }

private:
    ID3D11Buffer* buffer;
    unsigned int stride;
    unsigned int offset;
};
