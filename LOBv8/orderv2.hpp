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

    bool  inlist {};  // flag to avoid double entry of same order
};


// inherit from ListNode
struct Order : ListNode {

    OrderID  	external_id     {};	// belongs to market/gateway
    InternalID 	internal_id 	{};	// belongs to matching engine

    Price    price  {};
    Qty      qty    {};
    Side     side   { Side::Bid };

};

static_assert( std::is_trivially_copyable_v<Order> );
static_assert( std::is_trivially_destructible_v<Order> );
static_assert( sizeof( Order ) <= 64, "Order must fit in a single cache line\n" );
