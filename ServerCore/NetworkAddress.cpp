#include "pch.h"
#include "NetworkAddress.h"

NetworkAddress::NetworkAddress(SOCKADDR_IN sockAddr) : _sockAddr(sockAddr)
{
}

NetworkAddress::NetworkAddress(wstring IP, uint16 port)
{
	::memset(&_sockAddr, 0, sizeof(_sockAddr));

	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr = IPToAddress(IP.c_str());
	_sockAddr.sin_port = ::htons(port);
}

wstring NetworkAddress::GetIPAddress()
{
	const int32 BUFSIZE = 100;
	WCHAR buffer[BUFSIZE] = { 0, };

	::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, len32(buffer));

	return wstring();
}

IN_ADDR NetworkAddress::IPToAddress(const WCHAR* IP)
{
	IN_ADDR address = { 0, };
	::InetPtonW(AF_INET, IP, &address);
	return address;
}
