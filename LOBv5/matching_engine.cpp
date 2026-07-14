// matching engine src file// 04.07.26// ZeroK

#include "matching_engine.hpp"
#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"

#include <cinttypes>
#include <cstdio>

#include <immintrin.h>


void MatchingEngine::submit_order (Order* order) {

    Trade trade {};
    book_.add_order (order);


    while ( book_.match_order( trade ) ) {

        trade.timestamp_tsc =  __rdtsc();
        trade.trade_id      =  next_trade_id_++;

        std::printf ("TRADE:\n" 
                "Time    : %" PRIu64 "\n"
                "TradeID : %" PRIu64 "\n"
                "BuyID   : %" PRIu64 "\n"
                "SellID  : %" PRIu64 "\n"
                "Qty     : %" PRIu32 "\n"
                "Exec Px : %.2f\n"
                "Buy Px  : %.2f\n"
                "Sell Px : %.2f\n"
                "......................\n",
                trade.timestamp_tsc,
                trade.trade_id,
                trade.buy_id,
                trade.sell_id, 
                trade.qty,
                to_price (trade.exec_price),
                to_price (trade.buy_price),
                to_price (trade.sell_price)
            );
    }
}


// debuggin only
// void MatchingEngine::print_book() const noexcept {
//
//     book_.print_book();
// }

