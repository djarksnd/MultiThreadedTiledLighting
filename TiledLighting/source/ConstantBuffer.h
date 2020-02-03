#pragma once

#include <D3D11.h>

class ConstantBuffer
{
public:
	static constexpr unsigned int SizeOffset = 16;

public:
	bool Create(ID3D11Device*device, unsigned int size);
	void Destroy();

	bool Update(ID3D11DeviceContext*deviceContext, const void* data) const;

	operator ID3D11Buffer* () const { return buffer; }
	unsigned int GetSize() const { return size; }

	ConstantBuffer() : buffer(0), size(0) {}
	~ConstantBuffer() { Destroy(); }

private:
	ConstantBuffer(const ConstantBuffer&) = delete;
	const ConstantBuffer& operator = (const ConstantBuffer&) = delete;

private:
	ID3D11Buffer* buffer;
	unsigned int size;
};
