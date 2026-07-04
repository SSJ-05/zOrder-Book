// order generator header file// 27.06.26// ZeroK

#pragma once

#include "order.hpp"
#include <random>

class OrderGenerator {

private:
    std::uint64_t   id_;
    double          mid_price_;
    std::mt19937    rng_;

    std::uniform_int_distribution <int>            price_dist_;
    std::uniform_int_distribution <std::uint32_t>  qty_dist_;
    std::bernoulli_distribution side_dist_;

public:
    OrderGenerator() :
        id_ (0),
        mid_price_ (100.00),
        rng_ (std::random_device {} ()),
        price_dist_ (-2, 2),
        qty_dist_ (1, 100),
        side_dist_ (0.5) {}

    Order next ();
};

