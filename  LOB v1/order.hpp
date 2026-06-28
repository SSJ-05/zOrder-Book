// order header file// 27.06.26// ZeroK

#pragma once

#include <cstdint>
#include <type_traits>

// buy side, sell side in order book
enum class Side : std::uint8_t {
    Buy,
    Sell
};


struct Order {

    std::uint64_t  id;

    double         price;   // replace
    std::uint32_t  qty;

    Side           side;

    // char pad[];
};

static_assert (std::is_trivially_copyable_v<Order>);
