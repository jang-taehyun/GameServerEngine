#pragma once


/*------------------
	 Lock Queue
------------------*/

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WRITE_LOCK;
		_items.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();

		return ret;
	}

	void Clear()
	{
		WRITE_LOCK;
		_items = Queue<T>();
	}

	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
		{
			items.push_back(item);
		}
	}

private:
	USE_LOCK;
	Queue<T> _items;
};