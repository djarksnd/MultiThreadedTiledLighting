#pragma once

struct ResourceCPUAccessFlags
{
	bool read = false;
	bool write = false;

	enum BitFlag
	{
		readBit = 1,
		writeBit = 1 << 1,
	};

	ResourceCPUAccessFlags(unsigned int bitFlags)
	{
		read = bitFlags & readBit;
		write = bitFlags & writeBit;
	}

	ResourceCPUAccessFlags() {}
};
