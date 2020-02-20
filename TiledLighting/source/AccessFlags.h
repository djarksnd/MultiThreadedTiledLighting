#pragma once

struct AccessFlags
{
	bool read = false;
	bool write = false;

	enum BitFlag
	{
		readBit = 1,
		writeBit = 1 << 1,
	};

	AccessFlags(unsigned int bitFlags)
	{
		read = bitFlags & readBit;
		write = bitFlags & writeBit;
	}

	AccessFlags() {}
};
