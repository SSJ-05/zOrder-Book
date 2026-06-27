// order header file// 27.06.26// ZeroK

#pragma once

#include <cstdint>

// buy side, sell side in order book
enum class Side : std::uint8_t {
    BUY,
    SELL
}


struct Order {

    std::uint64_t  id;

    double         price;
    std::uint32_t  qty;

    Side           side;

    // char pad[];
};
