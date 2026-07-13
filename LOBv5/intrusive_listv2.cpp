// intrusive list definitions// 07.07.26// ZeroK

/* Invariants with sentinel nodes (head_ and tail_)
 * head_.next = first order, tail_.prev = last order
 * Empty     : head.next == &tail
 * One Node  : head.next == tail.prev
 * clear()   : should only be called on already empty level
 *             for O(1) to work
 * */


#include "intrusive_listv2.hpp"
#include "orderv2.hpp"

#include <cassert>

// init intrusive list ctor
IntrusiveList::IntrusiveList() {

    // link head to tail
    head_.next = &tail_;
    head_.prev = nullptr;

    // link tail to head
    tail_.prev = &head_;
    tail_.next = nullptr;
} 


void IntrusiveList::push_back (Order* order) {

    assert (order != nullptr);
    assert (!order->inlist);    // prevent double insertion

    assert (head_.next != nullptr);
    assert (tail_.prev != nullptr);
    assert (head_.next != order);
    assert (tail_.prev != order);


    // assumes tail.prev exists after ctor
    tail_.prev->next = order;       // link order
    order->prev      = tail_.prev;

    order->next = &tail_;           // link tail
    tail_.prev  = order;

    order->inlist = true;           // set the flag after append

    ++size_;
    // post append check
    assert (head_.next != &tail_);      // list is not empty
    assert (tail_.prev == order);       // order is last
}


// 5 cases
void IntrusiveList::erase (Order* order) {

    assert (order != nullptr);
    assert (order->inlist);     // must be in the list to erase

    assert (order->prev != nullptr);
    assert (order->next != nullptr);

    // case 1: empty list
    if (empty()) return;

    // rewire
    order->prev->next = order->next;
    order->next->prev = order->prev;

    --size_;

    // cleanup removed node
    order->prev   = nullptr;  
    order->next   = nullptr;  
    order->inlist = false;    // mark as removed 

     // post append check
    assert (head_.next != order);      
    assert (tail_.prev != order);        
}



void IntrusiveList::pop_front () {

    assert (!empty());

    erase( static_cast<Order*>( head_.next ) );

    --size_;
}


#ifndef NDEBUG
void IntrusiveList::validate() const {

    assert (head_.prev == nullptr);
    assert (tail_.next == nullptr);

    assert (head_.next != nullptr);
    assert (tail_.prev != nullptr);

    if (empty())
        assert (head_.next == &tail_);
}
#endif


bool IntrusiveList::empty() const noexcept {

    return head_.next == &tail_;
}


Order* IntrusiveList::front() const noexcept {

    assert (!empty());
    return static_cast<Order*>( head_.next );   // must return Order*
}


Order* IntrusiveList::back() const noexcept {

    assert (!empty());
    return static_cast<Order*>( tail_.prev );
}


std::size_t IntrusiveList::size() const noexcept {

    return size_;
}


// for traversals
Order* IntrusiveList::next ( Order* p ) const noexcept {

    if (p == nullptr) return nullptr;

    return (p->next == &tail_) 
                    ? nullptr 
                    : static_cast<Order*>( p->next );
}


// remove nodes b/w head and tail
void IntrusiveList::clear() noexcept {

    // link head to tail and vice versa
    head_.next = &tail_;
    tail_.prev = &head_;

    // reset size of list
    size_     = 0;
}
