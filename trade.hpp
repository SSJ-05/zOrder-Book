// trade header file// 27.06.26// ZeroK

#pragma once

#include <cstdint>

struct Trade {
    
    std::uint64_t   buy_id;
    std::uint64_t   sell_id;

    double          price;      // replace
    std::uint32_t   qty;
};

static_assert (std::is_trivially_copyable_v<Trade>);
