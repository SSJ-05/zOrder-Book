// price ladder/level// 05.07.26// ZeroK
// each price has a struct that contains
// all diff order ids of the same price

#pragma once

#include "types.hpp"
#include "order.hpp"
#include "intrusive_list.hpp"

#include <cstdint>

using LevelQty = Qty;


struct PriceLevel {

    explicit PriceLevel (Price p) :     // a price level shouldnt change
        price (p) {}

    Price     price      {};
    LevelQty  total_qty  {};

    IntrusiveList orders {};

};

