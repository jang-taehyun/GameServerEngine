#pragma once


/*---------------------
	 Buffer Reader
---------------------*/

class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; }
	uint32 ReadSize() { return _pos; }
	uint32 FreeSize() { return _size - _pos; }

	template<typename T>
	bool Peek(OUT T* dest) { return Peek(reinterpret_cast<void*>(dest), sizeof(T)); }
	bool Peek(OUT void* dest, uint32 len);

	template<typename T>
	bool Read(OUT T* dest) { return Read(reinterpret_cast<void*>(dest), sizeof(T)); }
	bool Read(OUT void* dest, uint32 len);

	template<typename T>
	BufferReader& operator>>(OUT T& dest)
	{
		dest = *(reinterpret_cast<T*>(&_buffer[_pos]));
		_pos += sizeof(T);
		return *this;
	}

private:
	BYTE* _buffer = nullptr;
	uint32 _size = 0;
	uint32 _pos = 0;

};

