// order header file// 07.07.26// ZeroK
// separate meta data (ListNode) from business data (Order)


#pragma once

#include "types.hpp"

#include <cstdint>
#include <type_traits>


// buy side, sell side in order book
enum class Side : std::uint8_t {
    Bid,
    Ask
};


struct ListNode {

    ListNode*  prev  { nullptr };
    ListNode*  next  { nullptr };

#ifndef NDEBUG
    bool     inlist { false };  // flag to avoid double entry of same order
#endif

};


struct Order : ListNode {

    OrderID  id     {};

    Qty      qty    {};

    Side     side   { Side::Bid };

};

static_assert (std::is_trivially_copyable_v<Order>);

