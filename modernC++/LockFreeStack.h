#ifndef LOCK_FREE_STACK_H
#define LOCK_FREE_STACK_H
#include<atomic>
#include<memory>
using std::atomic;

template<typename T>
class LockFreeStack {
public:
    LockFreeStack();
    T pop();
    void push(T);
    bool empty();
private:
    // Linked list of integers
    struct StackItem {
        StackItem(T val) : next(0), value(val) {}
        StackItem *next; // Next item, 0 if this is last
        T value;
    };

    struct StackHead {
        StackItem *link;      // First item, 0 if list empty
        unsigned count;      // How many times the list has changed (see lecture notes)
    };

    atomic<StackHead> head;
};

template<typename T>
LockFreeStack<T>::LockFreeStack()
{
    StackHead init;
    init.link = 0;
    init.count = 0;
    head.store(init);
}

// Pop value from stack
template<typename T>
T LockFreeStack<T>::pop()
{
    // What the head will be if nothing messed with it
    StackHead expected = head.load();
    StackHead newHead;
    bool succeeded = false;
    while(!succeeded) {
        newHead.link = expected.link->next;
        newHead.count = expected.count + 1;
        succeeded = head.compare_exchange_weak(expected, newHead);
    }
    T value = expected.link->value;
    delete expected.link;
    return value;
}

// Push an item onto the list with the given head
template<typename T>
void LockFreeStack<T>::push(T val)
{
    StackHead expected = head.load();
    StackItem *newItem = new StackItem(val);
    StackHead newHead;
    newHead.link = newItem;
    do {
        newItem->next = expected.link;
        newHead.count = expected.count - 1;
    } while(!head.compare_exchange_weak(expected, newHead));
}

template<typename T>
bool LockFreeStack<T>::empty(){
    StackHead expected = head.load();
    return expected.count == 0;
}

#endif