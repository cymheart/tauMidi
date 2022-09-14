package cymheart.tau.utils;

import java.util.ArrayList;
import java.util.Comparator;

public class ScLinkedList<T>
{
    private ScLinkedListNode<T> head = null;
    private ScLinkedListNode<T> tail = null;
    private int size = 0;

    /**是否使用内部数组*/
    private boolean isUseInnerArray = false;
    /**设置是否使用内部数组*/
    public void SetUseInnerArray(boolean use)
    {
        isUseInnerArray = use;
    }
    private ArrayList<ScLinkedListNode<T>> _array = new ArrayList<>();

    public ScLinkedList() {
    }

    public int Size(){
        return size;
    }

    public boolean Empty() {
        return (size == 0);
    }

    /**根据指定的标号找到对应的节点*/
    public ScLinkedListNode<T> GetNode(int idx)
    {
        if (idx <= 0) return head;
        else if (idx >= size - 1) return tail;

        int i = 0;
        ScLinkedListNode<T> node = head;
        for (; node != null; node = node.next) {
            if(i == idx)
                return node;
            i++;
        }

        return null;
    }

    /**根据指定的标号找到对应的元素*/
    public T GetElem(int idx)
    {
        if(isUseInnerArray)
        {
            if(_array.size() == 0)
                return null;

            if(idx <= 0) return _array.get(0).elem;
            else if (idx >= size - 1) return _array.get(size - 1).elem;
            return _array.get(idx).elem;
        }

        ScLinkedListNode<T> node = GetNode(idx);
        if(node == null)
            return null;
        return node.elem;
    }

    public void Clear()
    {
        head = tail = null;
        size = 0;
        _array.clear();
    }

    public ScLinkedListNode<T> GetHeadNode()
    {
        return head;
    }

    public ScLinkedListNode<T> GetLastNode()
    {
        return tail;
    }

    /**合并列表，并清空被合并list*/
    public void Merge(ScLinkedList<T> mergeList)
    {
        if(mergeList.Empty())
            return;

        if(isUseInnerArray) {
            if(mergeList.isUseInnerArray){
                _array.addAll(mergeList._array);
            } else {
                ScLinkedListNode<T> nd = mergeList.head;
                for (; nd != null; nd = nd.next)
                    _array.add(nd);
            }
        }

        //
        if(head == null)
        {
            head = mergeList.head;
            tail = mergeList.tail;
            size = mergeList.size;
            mergeList.Clear();
            return;
        }
        
        tail.next = mergeList.head;
        mergeList.head.prev = tail;
        tail = mergeList.tail;
        size += mergeList.size;
        mergeList.Clear();

    }

    // 往链表末尾加入节点
    public ScLinkedListNode<T> AddLast(T elem)
    {
        return AddBack(tail, elem);
    }

    // 往链表末尾加入节点
    public ScLinkedListNode<T> AddLast(T[] elems)
    {
        for(int i=0; i<elems.length; i++)
            AddBack(tail, elems[i]);
        return tail;
    }


    // 往链表末尾加入节点
    public ScLinkedListNode<T> AddLast(ScLinkedListNode<T> node)
    {
        return AddBack(tail, node);
    }

    // 往链表末尾加入节点
    public ScLinkedListNode<T> AddLast(ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode, int count)
    {
        return AddBack(tail, startNode, endNode, count);
    }

    // 往链表头加入节点
    public ScLinkedListNode<T> AddFirst(T elem)
    {
        return AddBefore(head, elem);
    }

    // 往链表头加入节点
    public ScLinkedListNode<T> AddFirst(ScLinkedListNode<T> node)
    {
        return AddBefore(head, node);
    }

    // 往链表头加入节点
    public ScLinkedListNode<T> AddFirst(ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode, int count)
    {
        return AddBefore(head, startNode, endNode, count);
    }

    public ScLinkedListNode<T> AddBack(ScLinkedListNode<T> prev, T elem)
    {
        ScLinkedListNode<T> node = new ScLinkedListNode<>(elem);
        AddBack(prev, node);
        return node;
    }

    public ScLinkedListNode AddBack(ScLinkedListNode prev, ScLinkedListNode node)
    {
        return AddBack(prev, node, node, 1);
    }

    public ScLinkedListNode<T> AddBack(ScLinkedListNode<T> prev, ScLinkedList<T> addList)
    {
        return AddBack(prev, addList.GetHeadNode(),addList.GetLastNode(), addList.Size());
    }

    public ScLinkedListNode<T> AddBack(
            ScLinkedListNode<T> prev, ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode, int count)
    {
        if(startNode == null || endNode == null)
            return null;

        if(isUseInnerArray) {
            //增加item到数组
            _AddToArrayBack(prev, startNode, endNode);
        }

        if(head == null)
        {
            head = startNode;
            tail = endNode;
            startNode.prev = null;
            endNode.next = null;
            size = count;
            return startNode;
        }

        if(prev == null)
            prev = tail;

        ScLinkedListNode<T> next = null;
        if(prev != null) {
            next = prev.next;
            prev.next = startNode;
        }

        endNode.next = next;
        startNode.prev = prev;

        if(next != null)
            next.prev = endNode;

        if(prev == tail)
            tail = endNode;

        size += count;
        return startNode;
    }


    private void _AddToArrayBack(ScLinkedListNode<T> prev, ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode)
    {
        int prevIdx = -1;
        if(prev != null)
            prevIdx = _array.indexOf(prev.elem);

        if(startNode == endNode)
        {
            if(prevIdx == -1) _array.add(startNode);
            else _array.add(prevIdx, startNode);
            return;
        }

        ScLinkedListNode<T> nd = startNode;
        ArrayList<ScLinkedListNode<T>> newArr = new ArrayList<>();
        for(;nd != null;nd = nd.next) {
            newArr.add(nd);
            if(nd == endNode)
                break;
        }

        if(prevIdx == -1) _array.addAll(newArr);
        else _array.addAll(prevIdx, newArr);
    }


    public ScLinkedListNode<T> AddBefore(ScLinkedListNode<T> next, T elem)
    {
        ScLinkedListNode<T> node = new ScLinkedListNode<>(elem);
        AddBefore(next, node);
        return node;
    }

    public ScLinkedListNode<T> AddBefore(ScLinkedListNode<T> next, ScLinkedListNode<T> node)
    {
       return AddBefore(next, node, node, 1);
    }

    public ScLinkedListNode<T> AddBefore(ScLinkedListNode<T> next, ScLinkedList<T> addList)
    {
        return AddBefore(next, addList.GetHeadNode(),addList.GetLastNode(), addList.Size());
    }

    public ScLinkedListNode<T> AddBefore(
            ScLinkedListNode<T> next, ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode, int count)
    {
        if(startNode == null || endNode == null)
            return null;

        if(isUseInnerArray)
            _AddToArrayBefore(next, startNode, endNode);

        if(head==null)
        {
            head = startNode;
            tail = endNode;
            startNode.prev = null;
            endNode.next = null;
            size = count;
            return startNode;
        }


        if(next == null)
            next = head;

        ScLinkedListNode<T> prev = null;
        if(next != null) {
            prev = next.prev;
            next.prev = endNode;
        }

        startNode.prev = prev;
        endNode.next = next;

        if(prev != null)
            prev.next = startNode;

        if(next == head)
            head = startNode;

        size++;

        return startNode;
    }


    private void _AddToArrayBefore(ScLinkedListNode<T> next, ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode)
    {
        ScLinkedListNode<T> prev = next.prev;
        ArrayList<ScLinkedListNode<T>> newArr = new ArrayList<>();

        if(prev == null)
        {
            if(startNode == endNode)
                newArr.add(startNode);
            else {
                ScLinkedListNode<T> nd = startNode;
                for (; nd != null; nd = nd.next) {
                    newArr.add(nd);
                    if (nd == endNode)
                        break;
                }
            }

            newArr.addAll(_array);
            _array.clear();
            _array.addAll(newArr);
            return;
        }

        _AddToArrayBack(prev, startNode, endNode);
    }

    public boolean Contains(T elem)
    {
       return (ContainsAtNode(elem) != null ? true:false);
    }

    public ScLinkedListNode<T> ContainsAtNode(T elem)
    {
        ScLinkedListNode<T> nd = head;
        for (; nd != null; nd = nd.next) {
            if (nd.elem == elem)
                return nd;
        }
        return null;
    }

    public boolean Remove(T elem)
    {
        ScLinkedListNode<T> nd = ContainsAtNode(elem);
        if(nd == null)
            return false;
        Remove(nd);
        return true;
    }

    public ScLinkedListNode<T> Remove(ScLinkedListNode<T> node)
    {
        return Remove(node, node, 1);
    }

    public ScLinkedListNode<T> Remove(ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode, int count)
    {
        if(startNode == null || endNode == null)
            return null;

        if((startNode.prev == null && startNode != head) ||
                (endNode.next == null && endNode != tail))
            return null;

        if(isUseInnerArray)
            _RemoveFromArray(startNode, endNode);

        ScLinkedListNode<T> prev = startNode.prev;
        ScLinkedListNode<T> next = endNode.next;

        if(prev != null)
            prev.next = next;
        else
            head = next;

        if(next != null)
            next.prev = prev;
        else
            tail = prev;

        size -= count;

        return next;
    }

    private void _RemoveFromArray(ScLinkedListNode<T> startNode, ScLinkedListNode<T> endNode)
    {
        if(startNode == endNode)
        {
            _array.remove(startNode);
            return;
        }

        ScLinkedListNode<T> nd = startNode;
        ArrayList<ScLinkedListNode<T>> newArr = new ArrayList<>();
        for(;nd != null;nd = nd.next) {
            newArr.add(nd);
            if(nd == endNode)
                break;
        }

        _array.removeAll(newArr);
    }


    private void _CopyToArray()
    {
        _array.clear();
        ScLinkedListNode<T> nd = head;
        for(;nd != null;nd = nd.next)
            _array.add(nd);
    }


    public void Sort(Comparator<T> cmp)
    {
        if(size <= 1)
            return;

        head.prev = _start;
        _start.next = head;
        tail.next = _end;
        _end.prev = tail;

        _refHead.value = head;
        _Sort(_refHead, size, cmp);
        head = _refHead.value;

        tail = _end.prev;
        head.prev = null;
        tail.next = null;

        if(isUseInnerArray)
            _CopyToArray();
    }


    private RefNode<T> _refHead = new RefNode<>(head);
    private ScLinkedListNode<T> _start = new ScLinkedListNode<>();
    private ScLinkedListNode<T> _end= new ScLinkedListNode<>();

    private static class RefNode<T>
    {
        public RefNode(ScLinkedListNode<T> v)
        {
            value = v;
        }
        ScLinkedListNode<T> value;
    }



    private static<T> ScLinkedListNode<T> _Sort(RefNode<T> _First, int _Size,  Comparator<T> cmp)
    {

        switch (_Size) {
            case 0:
                return _First.value;
            case 1:
                return _First.value.next;
            default:
                break;
        }

        ScLinkedListNode<T> _Mid = _Sort(_First, _Size / 2, cmp);
        RefNode<T> refMid = new RefNode<>(_Mid);
        ScLinkedListNode<T> _Last = _Sort(refMid, _Size - _Size / 2, cmp);
        _First.value = _Merge_same(_First.value, refMid.value, _Last, cmp);
        return _Last;
    }


    private static<T> ScLinkedListNode<T> _Merge_same(
            ScLinkedListNode<T> _First, ScLinkedListNode<T> _Mid, ScLinkedListNode<T> _Last,  Comparator<T> cmp)
    {
        ScLinkedListNode<T> _Newfirst;
        if (cmp.compare(_Mid.elem, _First.elem) == 1) {
            _Newfirst = _Mid;
        }
        else {
            _Newfirst = _First;
            do {
                _First = _First.next;
                if (_First == _Mid) {
                    return _Newfirst;
                }
            } while (cmp.compare(_Mid.elem, _First.elem) == 0);
        }

        for (;;)
        {
            ScLinkedListNode<T> _Run_start = _Mid;
            do {
                _Mid = _Mid.next;
            } while (_Mid != _Last && cmp.compare(_Mid.elem, _First.elem) == 1);

            _Splice(_First, _Run_start, _Mid);

            if (_Mid == _Last)
                return _Newfirst;

            do {
                _First = _First.next;
                if (_First == _Mid)
                    return _Newfirst;
            } while (cmp.compare(_Mid.elem, _First.elem) == 0);
        }
    }


    private static <T> ScLinkedListNode<T> _Splice(
            ScLinkedListNode<T> _Before, ScLinkedListNode<T> _First, ScLinkedListNode<T> _Last)
    {

        ScLinkedListNode<T>  _First_prev = _First.prev;
        _First_prev.next = _Last;
        ScLinkedListNode<T> _Last_prev = _Last.prev;
        _Last_prev.next = _Before;
        ScLinkedListNode<T> _Before_prev = _Before.prev;
        _Before_prev.next = _First;
        _Before.prev = _Last_prev;
        _Last.prev = _First_prev;
        _First.prev = _Before_prev;

        return _Last;
    }


}