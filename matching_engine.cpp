// matching engine src file// 27.06.26// ZeroK

#include "matching_engine.hpp"
#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"

#include <cinttypes>
#include <cstdio>

void MatchingEngine::submit_order (const Order& order) {

    book_.add_order (order);

    Trade trade;

    while (book_.match_order(trade)) {
        std::printf ("TRADE:\n" 
                "BuyID  : %" PRIu64 "\n"
                "SellID : %" PRIu64 "\n"
                "Qty    : %" PRIu32 "\n"
                "Ex Px  : %.2f\n"
                "Bx Px  : %.2f\n"
                "Sx Px  : %.2f\n"
                "......................\n",
                trade.buy_id, trade.sell_id, 
                trade.qty, trade.exec_price,
                trade.buy_price, trade.sell_price);
    }
}

// debuggin only
void MatchingEngine::print_book () const noexcept {

    book_.print_book();
}
