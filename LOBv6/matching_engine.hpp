// matching engine header file// 04.07.26// ZeroK

#pragma once

#include "orderbook.hpp"

class MatchingEngine {

private:
    Orderbook book_;

    OrderID next_trade_id_ { 1 };

public:
    void submit_order (Order*);

    void print_book () const noexcept;
};

