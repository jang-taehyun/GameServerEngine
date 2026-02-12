#pragma once

enum
{
	S_TEST = 1,
};


/*-----------------------------
	 Client Packet Handler
-----------------------------*/

class ClientPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);

private:
	static void Handle_S_TEST(BYTE* buffer, int32 len);
};