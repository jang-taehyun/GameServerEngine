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

    std::wstring name;
};

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
    using namespace std;

    BufferReader br{ buffer, (uint32)len };
    PacketHeader header;
    br >> header;

    uint64 ID = 0;
    uint32 hp = 0;
    uint16 attack = 0;
    br >> ID >> hp >> attack;

    cout << "ID : " << ID << ", hp : " << hp << ", attack : " << attack << endl;

    // 가변 데이터는 크기(size)를 먼저 받고,
    // 이후에 데이터를 쭉 받는다
    uint16 buffCount = 0;
    br >> buffCount;

    std::vector<BuffData> buffs(buffCount);
    for (int32 i = 0; i < buffCount; ++i)
        br >> buffs[i].buffID >> buffs[i].remainTime;

    cout << "BufCount : " << buffCount << endl;
    for (BuffData& buf : buffs)
        cout << "BufInfo : " << buf.buffID << ", " << buf.remainTime << endl;

    // 문자열 수신(UTF-16) //
    wstring name;
    uint16 nameLen = 0;
    br >> nameLen;

    name.resize(nameLen);
    br.Read((void*)name.data(), nameLen * sizeof(WCHAR));
    
    // wcout의 언어 설정
    wcout.imbue(std::locale("kor"));
    wcout << name << endl;
}

// packet 설계시 반드시 명심해야 하는 법칙 //
// -> 클라이언트는 항상 신용할 수 없다!!