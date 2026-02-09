#pragma once

enum
{
    S_TEST = 1,
};

// 패킷 설계 TEMP
struct BuffData
{
    uint64 buffID = 0;
    float remainTime = 0.f;
};

struct S_TEST
{
    uint64 ID = 0;
    uint32 HP = 0;
    uint16 attack = 0;

    // 가변 데이터
    std::vector<BuffData> buffs;
};


/*-----------------------------
     Server Packet Handler
-----------------------------*/

class ServerPacketHandler
{
public:
    static void HandlePacket(BYTE* buffer, int32 len);

    static SendBufferRef Make_S_TEST(uint64 ID, uint32 HP, uint16 attack, std::vector<BuffData> buffs);
};