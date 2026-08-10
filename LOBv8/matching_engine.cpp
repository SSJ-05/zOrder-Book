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

#include <cinttypes>
#include <cstdio>

#include <immintrin.h>



void MatchingEngine::submit_order ( OrderID external_id,
				    Price price, 
				    Qty qty,
				    Side side ) {

    ++submitted_;
    InternalID id =
	    book_.create_order( external_id, price, 
			    	qty, side );

    if ( id == zerok::OrderStore::INVALID_ID ) return;	  // book full


    while ( true ) {

	MatchResult result = {};

	if ( !book_.match_order( result ) ) break;

        result.trade.timestamp_tsc =  __rdtsc();
        result.trade.trade_id      =  next_trade_id_++;


#ifndef NDEBUG
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
#endif

	// if aggressive order is fully filled, cancel and release it
	Order& order  =  book_.order( id );

	if ( order.qty == 0 ) {
		if ( book_.cancel_order( id ) )
			++cancelled_;

		break;		// order is done, exit matching loop
	}
	// else: order remains in the book with remaining qty

    }	// while(true)
}




std::size_t MatchingEngine::book_size() const noexcept {
	
	return book_.size();
}



void MatchingEngine::print_stats() const noexcept {

    std::printf(
        "Submitted     : %zu\n"
        "Cancelled     : %zu\n",
        submitted_,
        cancelled_
	);
}

// debuggin only
void MatchingEngine::print_book() const noexcept {

    book_.print_book();
}

