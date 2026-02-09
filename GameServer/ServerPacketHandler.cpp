#include "pch.h"
#include "ServerPacketHandler.h"
#include "Session.h"
#include "BufferReader.h"
#include "BufferWriter.h"

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

SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 ID, uint32 HP, uint16 attack, std::vector<BuffData> buffs)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw{ sendBuffer->Buffer(), sendBuffer->AllocSize() };
	PacketHeader* header = bw.Reserve<PacketHeader>();

	// ID(uint64), 체력(uint32), 공격력(uint16)
	bw << ID << HP << attack;

	// 가변 데이터
	bw << (uint16)buffs.size();
	for (BuffData& buff : buffs)
	{
		bw << buff.buffID << buff.remainTime;
	}

	header->size = bw.WriteSize();
	header->ID = S_TEST;			// 1 : Test Msg

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}
