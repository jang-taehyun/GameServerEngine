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

// 패킷 구조
// [ PKT_S_TEST ][ BuffsListItem BuffsListItem BuffsListItem ... ][ victim victim ... ][ victim victim ... ]...[ victim victim ... ]

struct PKT_S_TEST
{
    struct BuffListItem
    {
        uint64 buffID = 0;
        float remainTime = 0.f;

        // Victim List
        uint16 victimsOffset = 0;
        uint16 victimsCount = 0;

        // 가변 길이 데이터(victims) 검사 //
        bool Validate(BYTE* packetStart, uint16 packetSize, OUT uint32& size)
        {
            uint16 cmp = victimsOffset + (victimsCount * sizeof(uint64));
            if (cmp > packetSize)
                return false;

            size += (victimsCount * sizeof(uint64));
            return true;
        }
    };

    uint16 packetSize = 0;      // 공용 헤더(PacketHeader)
    uint16 packetID = 0;        // 공용 헤더(PacketHeader)
    uint64 ID = 0;
    uint32 HP = 0;
    uint16 attack = 0;

    uint16 buffsOffset = 0;
    uint16 buffsCount = 0;

    bool IsValidate()
    {
        uint32 size = 0;
        size += sizeof(PKT_S_TEST);
        if (size > packetSize)
            return false;

        // 가변 길이 데이터(buffs) 검사 //

        // offset 검사
        if (buffsOffset + (buffsCount * sizeof(BuffListItem)) > packetSize)
            return false;

        // count 검사
        BuffsList buffList = GetBuffsList();
        size += sizeof(BuffListItem) * buffsCount;
        for (int32 i = 0; i < buffList.Count(); ++i)
        {
            if (false == buffList[i].Validate((BYTE*)this, packetSize, OUT size))
                return false;
        }


        // 전체 패킷 크기 검사 //
        if (size != packetSize)
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

    using BuffsVictimsList = PacketList<uint64>;
    BuffsVictimsList GetBuffsVictimList(BuffListItem* buffsItem)
    {
        BYTE* data = reinterpret_cast<BYTE*>(this);
        data += buffsItem->victimsOffset;
        return BuffsVictimsList{ reinterpret_cast<uint64*>(data), buffsItem->victimsCount };
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
    for (auto& buff : buffs)
    {
        cout << "BufInfo : " << buff.buffID << ", " << buff.remainTime << endl;

        PKT_S_TEST::BuffsVictimsList victims = pkt->GetBuffsVictimList(&buff);
        cout << "victim count : " << victims.Count() << endl;
        for (auto& victim : victims)
            cout << "victim : " << victim << endl;
    }
}

// packet 설계시 반드시 명심해야 하는 법칙 //
// -> 클라이언트는 항상 신용할 수 없다!!