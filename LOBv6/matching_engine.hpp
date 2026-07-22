// matching engine header file// 21.07.26// ZeroK

#pragma once

#include "orderbook.hpp"
#include "order_pool.hpp"


class MatchingEngine {

private:
    Orderbook   book_;
    OrderPool&  pool_;

    OrderID next_trade_id_ { 1 };

public:
    explicit MatchingEngine (OrderPool& pool)
	    : pool_ (pool) {}

    void submit_order (Order*);

    void print_book () const noexcept;
};

