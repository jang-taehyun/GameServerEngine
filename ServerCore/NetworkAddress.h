#pragma once

/*-----------------------
	 NetworkAddress
-----------------------*/

using std::wstring;

class NetworkAddress
{
public:
	NetworkAddress() = default;
	NetworkAddress(SOCKADDR_IN sockAddr);
	NetworkAddress(wstring IP, uint16 port);

	SOCKADDR_IN&		GetSocketAddress()		{ return _sockAddr; }
	wstring				GetIPAddress();
	uint16				GetPort()				{ return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR IPToAddress(const WCHAR* IP);

private:
	SOCKADDR_IN _sockAddr = { 0, };

};

