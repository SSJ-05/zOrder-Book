// order header file// 05.07.26// ZeroK

#pragma once

#include "types.hpp"

#include <type_traits>

// buy side, sell side in order book
enum class Side : std::uint8_t {
    Bid,
    Ask
};


// Order is node in intrusive list
struct Order {

    OrderID  id     {};

    Qty      qty    {};

    Side     side   { Side::Bid };

    // meta data
    Order*   prev   { nullptr };
    Order*   next   { nullptr };

    bool     inlist { false };  // flag to avoid double entry of same order
};

static_assert (std::is_trivially_copyable_v<Order>);

