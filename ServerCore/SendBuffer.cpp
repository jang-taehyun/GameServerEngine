#include "pch.h"
#include "SendBuffer.h"


/*------------------
	 Send Buffer
------------------*/

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}


/*------------------------
	 Send Buffer Chunk
------------------------*/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(SEND_BUFFER_CHUNK_SIZE >= allocSize);
	ASSERT_CRASH(false == _open);

	if (FreeSize() < allocSize)
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(true == _open);
	_open = false;
	_usedSize += writeSize;
}


/*--------------------------
	 Send Buffer Manager
--------------------------*/

// chunk 내에서 size만큼 메모리를 가져오는 함수
SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}
		
	ASSERT_CRASH(false == LSendBufferChunk->IsOpen());

	// 다 썼으면 버리고 새거로 교체
	if (size > LSendBufferChunk->FreeSize())
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	// using namespace std;
	// cout << "FREE : " << LSendBufferChunk->FreeSize() << endl;

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	// using namespace std;
	// cout << "Pop SENDBUFFERCHUNK" << endl;

	{
		WRITE_LOCK;
		if (false == _sendBufferChunks.empty())
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	using namespace std;
	cout << "PushGlobal SENDBUFFERCHUNK" << endl;

	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}