#include "pch.h"
#include "BufferReader.h"


/*---------------------
	 Buffer Reader
---------------------*/

BufferReader::BufferReader()
{
}

BufferReader::BufferReader(BYTE* buffer, uint32 size, uint32 pos)
	: _buffer(buffer), _size(size), _pos(pos)
{
}

BufferReader::~BufferReader()
{
}

bool BufferReader::Peek(OUT void* dest, uint32 len)
{
	if (FreeSize() < len)
		return false;

	::memcpy(dest, &_buffer[_pos], len);
	return true;
}

bool BufferReader::Read(OUT void* dest, uint32 len)
{
	if (false == Peek(dest, len))
		return false;

	_pos += len;
	return true;
}
