// matching engine src file// 04.07.26// ZeroK
/*
OrderPool
    │ creates
    ▼
MatchingEngine
    │ submits
    ▼
OrderBook
    │ matches
    ▼
MatchResult
    │ reports released orders
    ▼
MatchingEngine
    │
    ▼
OrderPool
*/

#include "matching_engine.hpp"
#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "order_pool.hpp"

#include <cinttypes>
#include <cstdio>

#include <immintrin.h>


void MatchingEngine::submit_order (Order* order) {

    MatchResult result;
    book_.add_order (order);


    while ( book_.match_order( result ) ) {

	assert( result.released_count <= 2 );
	result.released_count      =  0;
        result.trade.timestamp_tsc =  __rdtsc();
        result.trade.trade_id      =  next_trade_id_++;

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
                result.trade.timestamp_tsc,
                result.trade.trade_id,
                result.trade.buy_id,
                result.trade.sell_id, 
                result.trade.qty,
                to_price (result.trade.exec_price),
                to_price (result.trade.buy_price),
                to_price (result.trade.sell_price)
            );

	// release matched orders back to pool
	// **release order irrelevant - backward loop produced fewer asm insts.
	// for (auto i {}; i < result.released_count; ++i) {
	for (auto i {result.released_count}; i-- > 0;) {
		
		pool_.release( result.released [i] );
	}
    }

}


// debuggin only
// void MatchingEngine::print_book() const noexcept {
//
//     book_.print_book();
// }

