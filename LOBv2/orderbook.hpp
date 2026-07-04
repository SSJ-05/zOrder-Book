// order book header file// 04.07.26// ZeroK

#pragma once

#include "types.hpp"
#include "order.hpp"
#include "trade.hpp"
#include "price_level.hpp"

#include <map>
#include <limits>


class Orderbook {

private:
    std::map <Price, PriceLevel, std::greater<>> bids_;  // sort in descending order 
    std::map <Price, PriceLevel> asks_;                  // sort in ascending order

public:
    void add_order (const Order& order);

    bool match_order (Trade& trade);

    void print_book () const noexcept;
};

