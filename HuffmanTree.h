```
#ifndef _HUFFMAN_H_
#define  _HUFFMAN_H_
#include "Heap.hpp"
#include <queue>
template<class T>
struct HuffmanNode
{
	HuffmanNode(const T& _data)
		: _pLeft(NULL)
		, _pRight(NULL)
		, _pParent(NULL)
		, _weight(_data)
	{}
	HuffmanNode<T>* _pLeft;
	HuffmanNode<T>* _pRight;
	HuffmanNode<T>* _pParent;
	T _weight;
};

template<class T>
class Compare
{
public:
	bool operator()(const T& left, const T& right)
	{
		return left->_weight < right->_weight;
	}
};

template <class T>
class Huffman
{
public:
	Huffman()
		: pRoot(NULL)
	{}
	Huffman(const T arr[], size_t size, const T& invalid)
	{
		_pRoot = _CreateHuffman(arr, size, invalid);
	}
	void LeverOrder()
	{
		if (NULL == _pRoot)
		{
			return;
		}
		HuffmanNode<T>* pCur = _pRoot;
		queue<HuffmanNode<T>*> q;
		q.push(pCur);
		while (!q.empty())
		{
			HuffmanNode<T>* pFront = q.front();
			cout << pFront->_weight << " ";
			q.pop();
			if (NULL != pFront->_pLeft)
			{
				q.push(pFront->_pLeft);
			}
			if (NULL != pFront->_pRight)
			{
				q.push(pFront->_pRight);
			}

		}
		cout << endl;

	}

	~Huffman()
	{
		Destory(_pRoot);
	}
	HuffmanNode<T>* GetRoot()
	{
		return _pRoot;
	}

private:
	HuffmanNode<T>* _CreateHuffman(const T arr[], size_t size, const T& invalid)
	{
		if (size == 0)
		{
			return NULL;
		}
		Heap<HuffmanNode<T>*, Compare<HuffmanNode<T>*>> hp;
		for(size_t idx = 0; idx < size; ++idx)
		{
			if (arr[idx] != invalid)
			{
				hp.Insert(new HuffmanNode<T>(arr[idx]));
			}
		}

		while(hp.Size() > 1)
		{
			HuffmanNode<T>* pLeft = hp.Top();  // 取得第一个最小的元素
			hp.Remove();
			HuffmanNode<T>* pRight = hp.Top(); // 取得第二个最小的元素
			hp.Remove();
			HuffmanNode<T>* pRoot = new HuffmanNode<T>(pLeft->_weight+pRight->_weight);
			pRoot->_pLeft = pLeft;
			pRoot->_pLeft->_pParent = pRoot;
			pRoot->_pRight = pRight;
			pRoot->_pRight->_pParent = pRoot;
			hp.Insert(pRoot);
		}
		return hp.Top();
	}
	void Destory(HuffmanNode<T>*& pRoot)
	{
		if (NULL != pRoot)
		{
			Destory(pRoot->_pLeft);
			Destory(pRoot->_pRight);
			delete pRoot;
			pRoot = NULL;
		}
	}
private:
	 HuffmanNode<T>* _pRoot;
};

#endif
```
