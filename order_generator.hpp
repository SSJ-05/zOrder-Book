// order generator header file// 27.06.26// ZeroK

#pragma once

#include "order.hpp"

class OrderGenerator {

private:
    std::uint64_t   id_;
    double          mid_price_;
    std::uint32_t   mid_qty;

    std::mt19937    rng_;
    std::uniform_int_distribution<int> ud_;

public:
    Order() :
        id_ (1),
        mid_price_ (100.00),
        mid_qty_ (42),
        rng_ (std::random_device {} ()),
        ud_ (-2, 2) {}

    Order next ();
};
