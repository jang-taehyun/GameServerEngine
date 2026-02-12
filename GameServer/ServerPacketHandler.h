#pragma once

#include "BufferReader.h"
#include "BufferWriter.h"
#include "Protocol.pb.h"

enum
{
    S_TEST = 1,
};


/*-----------------------------
     Server Packet Handler
-----------------------------*/

class ServerPacketHandler
{
public:
    static void HandlePacket(BYTE* buffer, int32 len);

	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt);
};

template<typename T>
SendBufferRef _MakeSendBuffer(T& pkt, uint16 pktID)
{
	const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
	const uint16 packetSize = dataSize + sizeof(PacketHeader);

	SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
	header->size = packetSize;
	header->ID = pktID;

	// pkt에 존재하는 데이터 직렬화 //
	ASSERT_CRASH(pkt.SerializeToArray(header + 1, dataSize));

	sendBuffer->Close(packetSize);
	return sendBuffer;
}