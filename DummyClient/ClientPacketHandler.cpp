#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"


/*-----------------------------
	 Client Packet Handler
-----------------------------*/

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
    BufferReader br{ buffer, (uint32)len };
    PacketHeader header;
    br >> header;

    switch (header.ID)
    {
    case S_TEST:
        Handle_S_TEST(buffer, len);
        break;

    default:
        break;
    }
}

// 패킷 설계 TEMP
#pragma pack(1)
struct PKT_S_TEST
{
    struct BuffListItem
    {
        uint64 buffID = 0;
        float remainTime = 0.f;
    };

    uint16 packetSize = 0;      // 공용 헤더(PacketHeader)
    uint16 packetID = 0;        // 공용 헤더(PacketHeader)
    uint64 ID = 0;
    uint32 HP = 0;
    uint16 attack = 0;

    uint16 buffsOffset = 0;
    uint16 buffsCount = 0;

    // std::vector<BuffData> buffs;
    // std::wstring name;

    bool IsValidate()
    {
        uint32 size = 0;

        size += sizeof(PKT_S_TEST);
        size += sizeof(BuffListItem) * buffsCount;

        // packet의 전체 크기 검사 //
        if (size != packetSize)
            return false;

        // offset 검사
        if (buffsOffset + (buffsCount * sizeof(BuffListItem)) > packetSize)
            return false;

        return true;
    }
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
    using namespace std;

    if (sizeof(PKT_S_TEST) > len)
        return;
    
    BufferReader br{ buffer, (uint32)len };
    PKT_S_TEST pkt;
    br >> pkt;

    if (false == pkt.IsValidate())
        return;

    // cout << "ID : " << ID << ", hp : " << hp << ", attack : " << attack << endl;

    std::vector<PKT_S_TEST::BuffListItem> buffs(pkt.buffsCount);
    uint16 buffCount = pkt.buffsCount;

    for (int32 i = 0; i < buffCount; ++i)
        br >> buffs[i];

    cout << "BufCount : " << buffCount << endl;
    for (PKT_S_TEST::BuffListItem& buf : buffs)
        cout << "BufInfo : " << buf.buffID << ", " << buf.remainTime << endl;
}

// packet 설계시 반드시 명심해야 하는 법칙 //
// -> 클라이언트는 항상 신용할 수 없다!!