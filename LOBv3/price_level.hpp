// price ladder/level// 04.07.26// ZeroK
// each price has a struct that contains
// all diff order ids of the same price

#pragma once

#include "types.hpp"
#include "order.hpp"

#include <cstdint>
#include <list>

using LevelQty = Qty;


struct PriceLevel {

    explicit PriceLevel (Price p) :     // a price level shouldnt change
        price (p) {}

    Price price;

    LevelQty total_qty {};

    std::list<Order> orders;

    // helper func for clean erase in orderbook
    bool empty() const noexcept {
        return orders.empty();
    }
};

