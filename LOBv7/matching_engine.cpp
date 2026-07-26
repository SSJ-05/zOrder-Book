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

    ++submitted_;

    MatchResult result;
    book_.add_order( order );


    while ( true ) {

    	// result.released_count  =  0;
	result = {};

	if ( !book_.match_order( result ) ) break;

	assert( result.released_count <= 2 );

        result.trade.timestamp_tsc =  __rdtsc();
        result.trade.trade_id      =  next_trade_id_++;


        // std::printf ("TRADE:\n" 
        //         "Time    : %" PRIu64 "\n"
        //         "TradeID : %" PRIu64 "\n"
        //         "BuyID   : %" PRIu64 "\n"
        //         "SellID  : %" PRIu64 "\n"
        //         "Qty     : %" PRIu32 "\n"
        //         "Exec Px : %.2f\n"
        //         "Buy Px  : %.2f\n"
        //         "Sell Px : %.2f\n"
        //         "......................\n",
        //         result.trade.timestamp_tsc,
        //         result.trade.trade_id,
        //         result.trade.buy_id,
        //         result.trade.sell_id, 
        //         result.trade.qty,
        //         to_price (result.trade.exec_price),
        //         to_price (result.trade.buy_price),
        //         to_price (result.trade.sell_price)
        //     );

	// release matched orders back to pool
	// **release order irrelevant - backward loop produced fewer asm insts.
	// for (auto i {}; i < result.released_count; ++i) {
	for (auto i {result.released_count}; i-- > 0;) {
		
		++released_;
		pool_.release( result.released [i] );
	}
	fully_matched_ += result.released_count;

	if (Order* o = book_.cancel_order( order->id )) {
		++cancelled_;
		++released_;
		pool_.release( o );
	}

	assert( !order->inlist );
	assert( order->next == nullptr );
	assert( order->prev == nullptr );

    }	// while(true)
	
}




std::size_t MatchingEngine::book_size() const noexcept {
	
	return book_.size();
}



void MatchingEngine::print_stats() const noexcept {

    std::printf(
        "Submitted     : %zu\n"
        "Fully matched : %zu\n"
        "Released      : %zu\n"
        "Cancelled     : %zu\n",
        submitted_,
        fully_matched_,
        released_,
        cancelled_);
}

// debuggin only
void MatchingEngine::print_book() const noexcept {

    book_.print_book();
}

