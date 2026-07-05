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

    Order*   prev   { nullptr };
    Order*   next   { nullptr };

    // char pad[];
};

static_assert (std::is_trivially_copyable_v<Order>);

