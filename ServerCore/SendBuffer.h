#pragma once

class SendBufferChunk;


/*------------------
	 Send Buffer
------------------*/

class SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer; }
	uint32 WriteSize() { return _writeSize; }
	void Close(uint32 writeSize);

private:
	BYTE* _buffer = nullptr;
	uint32 _allocSize = 0;
	uint32 _writeSize = 0;
	SendBufferChunkRef _owner = nullptr;
};


/*------------------------
	 Send Buffer Chunk
------------------------*/
/**
* 할당 받을때마다 메모리를 가져오는게 아니라,
* 큰 메모리 영역(Chunk)을 가져온 후, 거기서 나눠서 사용한다.
*/

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000,
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void Reset();
	SendBufferRef Open(uint32 allocSize);
	void Close(uint32 writeSize);

	bool IsOpen() { return _open; }
	BYTE* Buffer() { return &_buffer[_usedSize]; }
	uint32 FreeSize() { return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> _buffer = { 0, };
	bool _open = false;
	uint32 _usedSize = 0;
};


/*--------------------------
	 Send Buffer Manager
--------------------------*/

class SendBufferManager
{
public:
	SendBufferRef Open(uint32 size);

private:
	SendBufferChunkRef Pop();
	void Push(SendBufferChunkRef buffer);
	
	static void PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;

	Vector<SendBufferChunkRef> _sendBufferChunks;
};