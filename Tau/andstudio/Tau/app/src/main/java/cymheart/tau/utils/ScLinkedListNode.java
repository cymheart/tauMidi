package cymheart.tau.utils;

public class ScLinkedListNode<T> {
    public T elem;
    public ScLinkedListNode<T> next;
    public ScLinkedListNode<T> prev;
    public ScLinkedListNode(){

    }
    public ScLinkedListNode(T elem){
        this.elem = elem;
        next = null;
        prev = null;
    }

    public void Clear()
    {
        elem = null;
        next = null;
        prev = null;
    }
}