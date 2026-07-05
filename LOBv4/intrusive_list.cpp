// intrusive list definitions// 05.07.26// ZeroK

#include "intrusive_list.hpp"
#include "order.hpp"


void IntrusiveList::push_back (Order* order) {

}


void IntrusiveList::erase (Order* order) {

}


void IntrusiveList::pop_front () {

}


bool IntrusiveList::empty() const noexcept {

    return (head_ == nullptr && tail_ == nullptr);
}


Order* IntrusiveList::front() const noexcept {

    return head_;
}


Order* IntrusiveList::back() const noexcept {

    return tail_;
}

