// intrusive list definitions// 05.07.26// ZeroK

/* Invariants
 * Empty     : head == nullptr && tail == nullptr
 * Non Empty : head->prev == nullptr && tail->next == nullptr
 * One Node  : head == tail
 * */

#include "intrusive_list.hpp"
#include "order.hpp"


void IntrusiveList::push_back (Order* order) {

    if (empty()) {

    }
    else {
    }
}

// 5 cases
void IntrusiveList::erase (Order* order) {

    // case 1: empty list
    if (empty()) return;

    // case 2: one node
    else if (head_ == tail_) {
        head_ = nullptr;
        tail_ = nullptr;
        return;
    } 

    // case 3: head
    else if (order == head_) {
        head_ = head_->next;
        head_->prev = nullptr;
        return;
    }

    // case 4: tail
    else if (order == tail_) {
        tail_ = tail_->prev;
        tail_->next = nullptr;
        return;
    }


    // case 5: middle node
    else  {
        order->prev->next = order->next;
        order->next->prev = order->prev;
        return;
    }

}



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

