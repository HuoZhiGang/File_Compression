```
#ifndef _HEAP_H_
#define  _HEAP_H_
#include <cassert>  //assert
#include <vector> 
#include <iostream>
using namespace std;

// 仿函数小堆调用 
template <class T>
class Less
{
public:
	bool operator()(const T& left, const T& right)
	{
		return left < right;
	}
};

// 仿函数大堆调用
template <class T>
class Greater
{
public:
	bool operator()(const T& left, const T& right)
	{
		return left > right;
	}
};

// 模板参数的大小堆
template <class T, class Compare = Less<T>>
class Heap
{
public:
	Heap(){}

	Heap(const T arr[] , int size)
	{
		for (int i=0; i<size; ++i)
		{
			_heap.push_back(arr[i]);
		}
		int last = (size - 2)/2; 
		for (int i=last; last>=0; --last)
		{
			_AdjustDown(last, size);
		}
	}
	void Insert(const T data)
	{
		_heap.push_back(data);
		_AdjustUp(_heap.size());
	}

	T& Top()
	{
		assert(!_heap.empty());

		return _heap[0]; 
	}
	const T& Top()const
	{
		assert(!_heap.empty());

		return _heap[0]; 
	}
	void Remove()
	{
		int size = _heap.size();
		if (size > 1)
		{
			std::swap(_heap[0], _heap[size-1]);
			_heap.pop_back();
			_AdjustDown(0, _heap.size());
		}
		else
		{
			_heap.pop_back();
		}
	}
	size_t Size()const
	{
		return _heap.size();
	}
private:
	void _AdjustUp(int size)
	{
		int child = size - 1;
		int parent = (child-1)/2;
		while (child != 0)
		{
			// 孩子小于父亲
			if (Compare()(_heap[child], _heap[parent]))
			{
				std::swap(_heap[child], _heap[parent]);
				child = parent;
				parent = (child - 1 )/2;
			}
			else
			{
				break;
			}
		}
	}
	void _AdjustDown(int root, int size)
	{
		int child = root*2 + 1;
		int parent = root;

		while (child < size)
		{
			if (child+1 < size && Compare()(_heap[child+1], _heap[child]) )
			{
				child += 1;
			}
			// 小 less 
			if (Compare()(_heap[child], _heap[parent]))
			{
				std::swap(_heap[child], _heap[parent]);
				parent = child;
				child = child *2 + 1;
			}
			else
			{
				break;
			}
		}
	}
private:
	vector<T> _heap;
};
#endif _HEAP_H_
```
