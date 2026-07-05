// order header file// 27.06.26// ZeroK

#pragma once

#include "types.hpp"

#include <type_traits>

// buy side, sell side in order book
enum class Side : std::uint8_t {
    Unknown,
    Bid,
    Ask
};


struct Order {

    OrderID  id;

    Price    price;   
    Qty      qty;

    Side     side;

    // char pad[];
};

static_assert (std::is_trivially_copyable_v<Order>);
