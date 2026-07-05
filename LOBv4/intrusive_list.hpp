// intrusive list class// 05.07.26// ZeroK

#pragma once

#include "order.hpp"

class IntrusiveList {

private:
    Order*  head_  { nullptr };
    Order*  tail_  { nullptr };

public:
    IntrusiveList() = default;
    ~IntrusiveList() = default;

    void   push_back (Order*);

    void   erase (Order*);

    void   pop_front();

    bool   empty() const noexcept;

    Order* front() const noexcept;

    Order* back()  const noexcept;


    IntrusiveList (const InstrusiveList&) = delete;
    IntrusiveList& operator=(const InstrusiveList&) = delete;

    IntrusiveList (InstrusiveList&&) = delete;
    IntrusiveList& operator=(InstrusiveList&&) = delete;

};

