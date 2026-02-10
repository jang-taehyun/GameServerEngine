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
        if (size > packetSize)
            return false;

        size += sizeof(BuffListItem) * buffsCount;

        // packet의 전체 크기 검사 //
        if (size != packetSize)
            return false;

        // offset 검사
        if (buffsOffset + (buffsCount * sizeof(BuffListItem)) > packetSize)
            return false;

        return true;
    }

    using BuffsList = PacketList<PKT_S_TEST::BuffListItem>;
    BuffsList GetBuffsList()
    {
        BYTE* data = reinterpret_cast<BYTE*>(this);
        data += buffsOffset;
        return BuffsList(reinterpret_cast<BuffListItem*>(data), buffsCount);
    }
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
    using namespace std;

    BufferReader br{ buffer, (uint32)len };
    PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

    if (false == pkt->IsValidate())
        return;

    // cout << "ID : " << ID << ", hp : " << hp << ", attack : " << attack << endl;

    PKT_S_TEST::BuffsList buffs = pkt->GetBuffsList();

    cout << "BufCount : " << buffs.Count() << endl;
    for (int32 i = 0; i < buffs.Count(); ++i)
        cout << "BufInfo : " << buffs[i].buffID << ", " << buffs[i].remainTime << endl;

    for (auto it = buffs.begin(); it != buffs.end(); ++it)
        cout << "BufInfo : " << it->buffID << ", " << it->remainTime << endl;

    for (auto& buff : buffs)
        cout << "BufInfo : " << buff.buffID << ", " << buff.remainTime << endl;
}

// packet 설계시 반드시 명심해야 하는 법칙 //
// -> 클라이언트는 항상 신용할 수 없다!!