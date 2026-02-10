#pragma once

#include "BufferReader.h"
#include "BufferWriter.h"

enum
{
    S_TEST = 1,
};


/*-----------------------------
     Server Packet Handler
-----------------------------*/

class ServerPacketHandler
{
public:
    static void HandlePacket(BYTE* buffer, int32 len);
};


/*-----------------------
	 Packet Iterator
-----------------------*/

template<typename T, typename C>
class PacketIterator
{
public:
	PacketIterator(C& container, uint16 index) : _container(container), _index(index) {}

	bool operator!=(const PacketIterator& other) const { return _index != other._index; }
	const T& operator*() const { return _container[_index]; }
	T& operator*() { return _container[_index]; }
	T* operator->() { return &_container[_index]; }
	PacketIterator& operator++() { ++_index; return *this; }
	PacketIterator operator++(int32) { PacketIterator ret = *this; ++_index; return ret; }

private:
	C& _container;
	uint16 _index;
};


/*-------------------
	 Packet List
-------------------*/

template<typename T>
class PacketList
{
public:
	PacketList() = default;
	PacketList(T* data, uint16 count) : _data(data), _count(count) {}

	T& operator[](uint16 index)
	{
		ASSERT_CRASH(0 <= index && index < _count);
		return _data[index];
	}

	uint16 Count() { return _count; }

	// ranged-base for 지원
	PacketIterator<T, PacketList<T>> begin()
	{
		return PacketIterator<T, PacketList<T>>(*this, 0);
	}

	PacketIterator<T, PacketList<T>> end()
	{
		return PacketIterator<T, PacketList<T>>(*this, _count);
	}

private:
	T* _data = nullptr;
	uint16 _count = 0;
};


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

};


/*---------------------------
	 Packet S_TEST write
---------------------------*/

class PKT_S_TEST_WRITE
{
public:
	using BuffsListItem = PKT_S_TEST::BuffListItem;
	using BuffsList = PacketList<PKT_S_TEST::BuffListItem>;

	PKT_S_TEST_WRITE(uint64 ID, uint32 HP, uint16 attack)
	{
		_sendBuffer = GSendBufferManager->Open(4096);
		_bw = BufferWriter{ _sendBuffer->Buffer(), _sendBuffer->AllocSize() };

		_pkt = _bw.Reserve<PKT_S_TEST>();
		_pkt->packetSize = 0;		// To Fill

		_pkt->packetID = S_TEST;
		_pkt->ID = ID;
		_pkt->HP = HP;
		_pkt->attack = attack;

		_pkt->buffsOffset = 0;		// To Fill
		_pkt->buffsCount = 0;		// To Fill


	}

	BuffsList ReserveBuffsList(uint16 buffCount)
	{
		BuffsListItem* firstBuffsListItem = _bw.Reserve<BuffsListItem>(buffCount);
		_pkt->buffsOffset = (uint64)firstBuffsListItem - (uint64)_pkt;
		_pkt->buffsCount = buffCount;
		return BuffsList{ firstBuffsListItem, buffCount };
	}

	SendBufferRef CloseAndReturn()
	{
		// 패킷 사이즈 계산
		_pkt->packetSize = _bw.WriteSize();

		_sendBuffer->Close(_bw.WriteSize());
		return _sendBuffer;
	}

private:
	PKT_S_TEST* _pkt;
	SendBufferRef _sendBuffer;
	BufferWriter _bw;
};

#pragma pack()