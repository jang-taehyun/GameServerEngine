#include "pch.h"
#include "ServerPacketHandler.h"
#include "Session.h"

void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br{ buffer, (uint32)len };
	PacketHeader header;
	br.Peek(&header);

	switch (header.ID)
	{
	default:
		break;
	}
}

SendBufferRef ServerPacketHandler::MakeSendBuffer(Protocol::S_TEST& pkt)
{
	return _MakeSendBuffer(pkt, S_TEST);
}
