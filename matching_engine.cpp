// matching engine src file// 27.06.26// ZeroK

#include "matching_engine.hpp"
#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"


void MatchingEngine::submit (const Order& order) {

    book_.add_order (order);

    Trade trade;

    while (book_.match_order(order)) {
        std::printf ("TRADE: %u @ %.2f\n",
                    trade.qty, trade.price);
    }
}
