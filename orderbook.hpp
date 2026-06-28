// order book header file// 27.06.26// ZeroK

#pragma once

#include "order.hpp"
#include "trade.hpp"

#include <vector>
#include <limits>

class Orderbook {

private:
    std::vector<Order> bids_;
    std::vector<Order> asks_;
    
public:
    double best_bid   { std::numeric_limits<double>::lowest() };
    double best_ask   { std::numeric_limits<double>::max() };

    void add_order (const Order& order);

    bool match_order (Trade& trade);

    void print_book () const noexcept;
};

