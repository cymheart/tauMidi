#ifndef _LinkedList_h_
#define _LinkedList_h_

#include"LinkedListNode.h"

namespace scutils
{

	template<typename T>
	class LinkedList
	{

	public:

		int Size() {
			return size;
		}

		bool Empty() {
			return (size == 0);
		}

		/**根据指定的标号找到对应的节点*/
		LinkedListNode<T>* GetNode(int idx)
		{
			if (idx <= 0) return head;
			else if (idx >= size - 1) return tail;

			int i = 0;
			LinkedListNode<T>* node = head;
			for (; node; node = node->next) {
				if (i == idx)
					return node;
				i++;
			}

			return nullptr;
		}

		void Clear()
		{
			head = tail = nullptr;
			size = 0;
		}

		void Release()
		{
			LinkedListNode<T>* node = head;
			LinkedListNode<T>* next;
			for (; node; node = next)
			{
				next = node->next;
				delete node;
			}

			head = tail = nullptr;
			size = 0;
		}


		LinkedListNode<T>* GetHeadNode()
		{
			return head;
		}

		LinkedListNode<T>* GetLastNode()
		{
			return tail;
		}

		/**合并列表，并清空被合并list*/
		void Merge(LinkedList<T>& mergeList)
		{
			if (mergeList.Empty())
				return;

			if (head == nullptr)
			{
				head = mergeList.head;
				tail = mergeList.tail;
				size = mergeList.size;
				mergeList.Clear();
				return;
			}

			LinkedListNode<T>* node = mergeList.head;
			tail->next = mergeList.head;
			mergeList.head->prev = tail;
			tail = mergeList.tail;
			size += mergeList.size;
			mergeList.Clear();
		}

		// 往链表末尾加入节点
		LinkedListNode<T>* AddLast(T elem)
		{
			return AddBack(tail, elem);
		}

		// 往链表末尾加入节点
		LinkedListNode<T>* AddLast(LinkedListNode<T>* node)
		{
			return AddBack(tail, node);
		}


		// 往链表末尾加入节点
		LinkedListNode<T>* AddLast(LinkedListNode<T>* startNode, LinkedListNode<T>* endNode, int count)
		{
			return AddBack(tail, startNode, endNode, count);
		}


		// 往链表头加入节点
		LinkedListNode<T>* AddFirst(T elem)
		{
			return AddBefore(head, elem);
		}

		// 往链表头加入节点
		LinkedListNode<T>* AddFirst(LinkedListNode<T>* node)
		{
			return AddBefore(head, node);
		}

		// 往链表头加入节点
		LinkedListNode<T>* AddFirst(LinkedListNode<T>* startNode, LinkedListNode<T>* endNode, int count)
		{
			return AddBefore(head, startNode, endNode, count);
		}

		LinkedListNode<T>* AddBack(LinkedListNode<T>* prev, T elem)
		{
			LinkedListNode<T>* node = new LinkedListNode<T>(elem);
			AddBack(prev, node);
			return node;
		}

		LinkedListNode<T>* AddBack(LinkedListNode<T>* prev, LinkedListNode<T>* node)
		{
			return AddBack(prev, node, node, 1);
		}

		LinkedListNode<T>* AddBack(LinkedListNode<T>* prev, LinkedList<T>& addList)
		{
			return AddBack(prev, addList.GetHeadNode(), addList.GetLastNode(), addList.Size());
		}

		LinkedListNode<T>* AddBack(
			LinkedListNode<T>* prev, LinkedListNode<T>* startNode, LinkedListNode<T>* endNode, int count)
		{
			if (startNode == nullptr || endNode == nullptr)
				return nullptr;

			if (head == nullptr)
			{
				head = startNode;
				tail = endNode;
				startNode->prev = nullptr;
				endNode->next = nullptr;
				size = count;
				return startNode;
			}

			if (prev == nullptr)
				prev = tail;

			LinkedListNode<T>* next = nullptr;
			if (prev != nullptr) {
				next = prev->next;
				prev->next = startNode;
			}

			endNode->next = next;
			startNode->prev = prev;

			if (next != nullptr)
				next->prev = endNode;

			if (prev == tail)
				tail = endNode;

			size += count;
			return startNode;
		}


		LinkedListNode<T>* AddBefore(LinkedListNode<T>* next, T elem)
		{
			LinkedListNode<T>* node = new LinkedListNode<T>(elem);
			AddBefore(next, node);
			return node;
		}

		LinkedListNode<T>* AddBefore(LinkedListNode<T>* next, LinkedListNode<T>* node)
		{
			return AddBefore(next, node, node, 1);
		}

		LinkedListNode<T>* AddBefore(LinkedListNode<T>* next, LinkedList<T>& addList)
		{
			return AddBefore(next, addList.GetHeadNode(), addList.GetLastNode(), addList.Size());
		}

		LinkedListNode<T>* AddBefore(
			LinkedListNode<T>* next, LinkedListNode<T>* startNode, LinkedListNode<T>* endNode, int count)
		{
			if (startNode == nullptr || endNode == nullptr)
				return nullptr;

			if (head == nullptr)
			{
				head = startNode;
				tail = endNode;
				startNode->prev = nullptr;
				endNode->next = nullptr;
				size = count;
				return startNode;
			}

			if (next == nullptr)
				next = head;

			LinkedListNode<T>* prev = nullptr;
			if (next != nullptr) {
				prev = next->prev;
				next->prev = endNode;
			}

			startNode->prev = prev;
			endNode->next = next;

			if (prev != nullptr)
				prev->next = startNode;

			if (next == head)
				head = startNode;

			size++;

			return startNode;
		}

		bool Contains(T elem)
		{
			return (ContainsAtNode(elem) != nullptr ? true : false);
		}

		LinkedListNode<T>* ContainsAtNode(T elem)
		{
			LinkedListNode<T>* nd = head;
			for (; nd; nd = nd->next) {
				if (nd->elem == elem)
					return nd;
			}
			return nullptr;
		}

		bool Remove(T elem)
		{
			LinkedListNode<T>* nd = ContainsAtNode(elem);
			if (nd)
				return false;
			Remove(nd);
			return true;
		}

		LinkedListNode<T>* Remove(LinkedListNode<T>* node)
		{
			return Remove(node, node, 1);
		}


		LinkedListNode<T>* Remove(LinkedListNode<T>* startNode, LinkedListNode<T>* endNode, int count)
		{
			if (startNode == nullptr || endNode == nullptr)
				return nullptr;

			if ((startNode->prev == nullptr && startNode != head) ||
				(endNode->next == nullptr && endNode != tail))
				return nullptr;

			LinkedListNode<T>* prev = startNode->prev;
			LinkedListNode<T>* next = endNode->next;

			if (prev != nullptr)
				prev->next = next;
			else
				head = next;

			if (next != nullptr)
				next->prev = prev;
			else
				tail = prev;

			size -= count;

			return next;
		}

		template <class Fn>
		void Sort(Fn cmp)
		{
			if (size <= 1)
				return;

			LinkedListNode<T> start;
			head->prev = &start;
			start.next = head;

			LinkedListNode<T> end;
			tail->next = &end;
			end.prev = tail;

			_Sort(head, size, cmp);

			tail = end.prev;
			head->prev = nullptr;
			tail->next = nullptr;
		}

	private:


		template <class Fn>
		static LinkedListNode<T>* _Sort(LinkedListNode<T>*& _First, int _Size, Fn cmp)
		{
			switch (_Size) {
			case 0:
				return _First;
			case 1:
				return _First->next;
			default:
				break;
			}

			auto _Mid = _Sort(_First, _Size / 2, cmp);
			const auto _Last = _Sort(_Mid, _Size - _Size / 2, cmp);
			_First = _Merge_same(_First, _Mid, _Last, cmp);
			return _Last;
		}


		template <class Fn>
		static LinkedListNode<T>* _Merge_same(LinkedListNode<T>* _First, LinkedListNode<T>* _Mid, const LinkedListNode<T>* _Last, Fn cmp)
		{
			LinkedListNode<T>* _Newfirst;
			if (cmp(_Mid->elem, _First->elem)) {
				_Newfirst = _Mid;
			}
			else {
				_Newfirst = _First;
				do {
					_First = _First->next;
					if (_First == _Mid) {
						return _Newfirst;
					}
				} while (!cmp(_Mid->elem, _First->elem));
			}

			for (;;)
			{
				auto _Run_start = _Mid;
				do {
					_Mid = _Mid->next;
				} while (_Mid != _Last && cmp(_Mid->elem, _First->elem));

				_Splice(_First, _Run_start, _Mid);

				if (_Mid == _Last)
					return _Newfirst;

				do {
					_First = _First->next;
					if (_First == _Mid)
						return _Newfirst;
				} while (!cmp(_Mid->elem, _First->elem));
			}
		}


		static LinkedListNode<T>* _Splice(LinkedListNode<T>* _Before, LinkedListNode<T>* _First, LinkedListNode<T>* _Last)
		{

			// fixup the _Next values
			auto _First_prev = _First->prev;
			_First_prev->next = _Last;
			auto _Last_prev = _Last->prev;
			_Last_prev->next = _Before;
			auto _Before_prev = _Before->prev;
			_Before_prev->next = _First;

			// fixup the _Prev values
			_Before->prev = _Last_prev;
			_Last->prev = _First_prev;
			_First->prev = _Before_prev;

			return _Last;
		}

	private:
		LinkedListNode<T>* head = nullptr;
		LinkedListNode<T>* tail = nullptr;
		int size = 0;

	};
}

#endif
