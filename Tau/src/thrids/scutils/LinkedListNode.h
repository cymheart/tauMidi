#ifndef _LinkedListNode_h_
#define _LinkedListNode_h_

namespace scutils
{
	template<typename T>
	class LinkedListNode
	{
	public:
		LinkedListNode() {

		}

		LinkedListNode(T elem) {
			this->elem = elem;
			next = nullptr;
			prev = nullptr;
		}

		void Clear()
		{
			elem = nullptr;
			next = nullptr;
			prev = nullptr;
		}

	public:
		T elem;
		LinkedListNode<T>* next = nullptr;
		LinkedListNode<T>* prev = nullptr;
	};
}

#endif
