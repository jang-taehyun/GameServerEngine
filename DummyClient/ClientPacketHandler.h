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

	// ranged-base for Áö¿ø
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