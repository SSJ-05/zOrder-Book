// intrusive list class// 07.07.26// ZeroK

#pragma once

#include "orderv2.hpp"


class IntrusiveList {

private:
    ListNode  head_  { nullptr, nullptr };
    ListNode  tail_  { nullptr, nullptr };

    std::size_t size_ { 0uz };

public:
    IntrusiveList();
    ~IntrusiveList() = default;

    // all ops O(1)
    void   push_back (Order*);

    void   erase (Order*);

    void   pop_front();

    void   validate() const;

    bool   empty() const noexcept;

    Order* front() const noexcept;

    Order* back()  const noexcept;

    std::size_t size() const noexcept;

    // helper func for iteration
    Order* next (Order* p) const noexcept;

    // copy and move semantics
    IntrusiveList (const IntrusiveList&) = delete;
    IntrusiveList& operator=(const IntrusiveList&) = delete;

    IntrusiveList (IntrusiveList&&) = delete;
    IntrusiveList& operator=(IntrusiveList&&) = delete;

};

