// order book header file// 27.06.26// ZeroK

#pragma once

#include "order.hpp"
#include "trade.hpp"

#include <vector>

class Orderbook {

private:
    std::vector<Order> bids_;
    std::vector<Order> asks_;


public:
    void add_order (const Order& order);

    bool match_order (Order& order);

    void print_book () const noexcept;
};
