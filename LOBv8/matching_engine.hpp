// matching engine header file// 21.07.26// ZeroK

#pragma once

#include "orderbook.hpp"


class MatchingEngine {

private:

    Orderbook   book_;

    OrderID next_trade_id_ { 2 };

    // counters
    std::size_t  submitted_ {};
    std::size_t  fully_matched_ {};
    std::size_t  cancelled_ {};


public:

    // explicit MatchingEngine (OrderPool& pool)
    //  : pool_ (pool) {}

    void submit_order ( OrderID external_id,
		        Price price,
			Qty qty,
			Side side );

    [[ nodiscard ]] 
    std::size_t book_size() const noexcept;

    void print_stats() const noexcept;
    void print_book () const noexcept;
};

