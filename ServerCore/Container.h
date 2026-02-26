#pragma once

#include "Types.h"
#include "Allocator.h"

#include <array>
#include <vector>
#include <list>
#include <deque>

#include <queue>
#include <stack>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <string>

using std::array;
using std::vector;
using std::list;
using std::deque;

using std::queue;
using std::stack;
using std::priority_queue;

using std::map;
using std::set;
using std::unordered_map;
using std::unordered_set;

using std::basic_string;

template<typename T, uint32 Size>
using Array = array<T, Size>;

template<typename Type>
using Vector = vector<Type, STLAllocator<Type>>;

template<typename Type>
using List = list<Type, STLAllocator<Type>>;

template<typename Type>
using Deque = deque<Type, STLAllocator<Type>>;


template<typename Type, typename Container = Deque<Type>>
using Queue = queue<Type, Container>;

template<typename Type, typename Container = Deque<Type>>
using Stack = stack<Type, Container>;

template<typename Type, typename Container = Vector<Type>, typename Pred = std::less<typename Container::value_type>>
using PriorityQueue = priority_queue<Type, Container, Pred>;


template<typename Key, typename Type, typename Pred = std::less<Key>>
using Map = map<Key, Type, Pred, STLAllocator<std::pair<const Key, Type>>>;

template<typename Type, typename Pred = std::less<Type>>
using Set = set<Type, Pred, STLAllocator<Type>>;

template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using HashMap = unordered_map<Key, Type, Hasher, KeyEq, STLAllocator<std::pair<const Key, Type>>>;

template<typename Type, typename Hasher = std::hash<Type>, typename KeyEq = std::equal_to<Type>>
using HashSet = unordered_set<Type, Hasher, KeyEq, STLAllocator<Type>>;


// using String = basic_string<char, std::char_traits<char>, STLAllocator<char>>;
// using WString = basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>>;
using String = basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>>;