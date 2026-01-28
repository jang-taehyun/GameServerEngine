#pragma once

#include "Types.h"

#pragma region TypeList

template<typename... T>
struct TypeList;

template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion

#pragma region Length

template<typename T>
struct Length;

template<>
struct Length<TypeList<>>
{
	enum { value = 0 };
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value };
};

#pragma endregion

#pragma region TypeAt

template<typename TL, int32 index>
struct TypeAt;

template<typename Head, typename... Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

#pragma endregion

#pragma region IndexOf

template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum { value = 0 };
};

// type을 찾지 못한 경우 //
template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename Target>
struct IndexOf<TypeList<Head, Tail...>, Target>
{
private:
	enum { res = IndexOf<TypeList<Tail...>, Target>::value };

public:
	enum { value = (-1 == res ? -1 : res + 1) };
};

#pragma endregion

#pragma region Conversion

template<typename From, typename To>
class Conversion
{
	using Small		=	__int8;
	using Big		=	__int32;

public:
	static Small	Test		(const To&)		{ return 0; }
	static Big		Test		(...)			{ return 0; }
	static From		MakeFrom	()				{ return 0; }

	enum { exist = (sizeof(Test(MakeFrom())) == sizeof(Small)) };
};

#pragma endregion

#pragma region TypeCast

// 정수를 type으로 만들어주는 구조체 //
template<int32 V>
struct IntToType
{
	enum { value = V };
};

template<typename TL>
class TypeConversion
{
	enum { length = Length<TL>::value };

public:
	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion<TL> conversion;
		return s_convert[from][to];
	}

private:
	TypeConversion()
	{
		MakeTable(IntToType<0>(), IntToType<0>());
	}

	template<int32 column, int32 row>
	static void MakeTable(IntToType<column>, IntToType<row>)
	{
		using FromType	= typename TypeAt<TL, column>::Result;
		using ToType	= typename TypeAt<TL, row>::Result;

		if (1 == Conversion<FromType, ToType>::exist)
			s_convert[column][row] = true;

		MakeTable(IntToType<column>(), IntToType<row + 1>());
	}

	template<int32 column>
	static void MakeTable(IntToType<column>, IntToType<length>)
	{
		MakeTable(IntToType<column + 1>(), IntToType<0>());
	}

	template<int32 row>
	static void MakeTable(IntToType<length>, IntToType<row>)
	{

	}

private:
	static bool s_convert[length][length];
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length] = { false, };

template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (nullptr == ptr)
		return nullptr;

	using TL = typename From::TL;
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (nullptr == ptr)
		return nullptr;

	using TL = typename From::TL;
	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return std::static_pointer_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (nullptr == ptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (nullptr == ptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

#pragma endregion

#define DECLARE_TL	\
using TL = TL;		\
int32 _typeId;		\

#define INIT_TL(type)	_typeId = IndexOf<TL, type>::value