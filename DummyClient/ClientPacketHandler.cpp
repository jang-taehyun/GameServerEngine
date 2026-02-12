#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "Protocol.pb.h"


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

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
    using namespace std;

    Protocol::S_TEST pkt;

    BYTE* ptr = buffer + sizeof(PacketHeader);

    // 수신한 데이터를 역직렬화 //
    ASSERT_CRASH(pkt.ParseFromArray(ptr, len - sizeof(PacketHeader)));

    {
        cout << "-------------------------" << endl;
        cout << pkt.id() << ", " << pkt.hp() << ", " << pkt.attack() << endl;
        cout << "BUFSIZE : " << pkt.buffs_size() << endl;
        for (auto& buf : pkt.buffs())
        {
            cout << buf.buffid() << ", " << buf.remaintime() << endl;

            cout << "VICTIMS SIZE : " << buf.victims_size() << endl;
            for (auto& vic : buf.victims())
                cout << vic << ", ";
            cout << endl;
        }
        cout << "-------------------------" << endl;
    }
}
