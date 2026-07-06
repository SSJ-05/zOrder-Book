// intrusive list definitions// 05.07.26// ZeroK

/* Invariants
 * Empty     : head == nullptr && tail == nullptr
 * Non Empty : head->prev == nullptr && tail->next == nullptr
 * One Node  : head == tail
 * */

#include "intrusive_list.hpp"
#include "order.hpp"

#include <cassert>


void IntrusiveList::push_back (Order* order) {

    assert (order != nullptr);
    assert (!order->inlist);    // prevent double insertion

    order->prev = nullptr;      // make sure new node... 
    order->next = nullptr;      // ...doesnt point to anything

    if (empty()) {
        head_ = order;
        tail_ = order;

        order->inlist = true;
    }
    else {
        tail_->next = order;
        order->prev = tail_;
        tail_ = order;          // new tail
                                
        order->inlist = true;
    }
}


// 5 cases
void IntrusiveList::erase (Order* order) {

    assert (order != nullptr);
    assert (order->inlist);     // must be in the list to erase

    // case 1: empty list
    if (empty()) return;

    // case 2: one node
    if (head_ == tail_) {
        // rewire
        head_ = nullptr;
        tail_ = nullptr;
    } 

    // case 3: head
    if (order == head_) {
        // rewire
        head_ = head_->next;
        head_->prev = nullptr;
    }

    // case 4: tail
    if (order == tail_) {
        // rewire
        tail_ = tail_->prev;
        tail_->next = nullptr;
    }

    // case 5: middle node
    else  {
        // rewire
        order->prev->next = order->next;
        order->next->prev = order->prev;
    }

    // cleanup
    order->prev   = nullptr;  // make sure removed node... 
    order->next   = nullptr;  // ...doesnt point to anything
    order->inlist = false;    // mark as removed 

    return;
}

assert (head_ == nullptr || head_->prev == nullptr);
assert (tail_ == nullptr || tail_->next == nullptr);



void IntrusiveList::pop_front () {

    if (head_) erase (head_);
    else return;
}


bool IntrusiveList::empty() const noexcept {

    return head_ == nullptr;
}


Order* IntrusiveList::front() const noexcept {

    return head_;
}


Order* IntrusiveList::back() const noexcept {

    return tail_;
}

