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