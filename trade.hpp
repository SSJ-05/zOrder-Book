// trade header file// 27.06.26// ZeroK

#pragma once

#include <cstdint>

struct Trade {
    
    std::uint64_t   buy_id;
    std::uint64_t   sell_id;

    double          price;
    std::uint32_t   qty;
};
