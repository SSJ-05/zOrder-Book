// trade header file// 27.06.26// ZeroK

#pragma once

#include "types.hpp"
#include <type_traits>

struct Trade {
    
    OrderID   trade_id      {};

    std::uint64_t timestamp_tsc  {};

    OrderID   buy_id        {};
    OrderID   sell_id       {};

    Price    buy_price      {};
    Price    sell_price     {};

    Price    exec_price     {};     // execution price
    Qty      qty            {};

};

static_assert (std::is_trivially_copyable_v<Trade>);

