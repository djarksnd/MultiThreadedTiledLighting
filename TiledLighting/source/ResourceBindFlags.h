#pragma once

struct ResourceBindFlags
{
	bool RenderTarget = false;
	bool DepthStencil = false;
	bool ShaderResource = false;
	bool UnorderedAccess = false;

	enum BitFlag
	{
		RenderTargetBit = 1,
		DepthStencilBit = 1 << 1,
		ShaderResourceBit = 1 << 2,
		UnorderedAccessBit = 1 << 3,
	};

	ResourceBindFlags(unsigned int bitFlags)
	{
		RenderTarget = bitFlags & RenderTargetBit;
		DepthStencil = bitFlags & DepthStencilBit;
		ShaderResource = bitFlags & ShaderResourceBit;
		UnorderedAccess = bitFlags & UnorderedAccessBit;
	}

	ResourceBindFlags() {}
};
