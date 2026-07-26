// price ladder/level// 05.07.26// ZeroK
// each price has a struct that contains
// all diff order ids of the same price

#pragma once

#include "types.hpp"
#include "orderv2.hpp"
#include "intrusive_listv2.hpp"

#include <cstdint>

using LevelQty = Qty;

struct PriceLevel {

    Price     price      {};
    LevelQty  total_qty  {};

    IntrusiveList orders {};
};

