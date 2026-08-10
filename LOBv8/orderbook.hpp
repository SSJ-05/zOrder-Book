// order book header file// 10.08.26// ZeroK

/* workflow v8
 * orderbook -> orderstore -> internal ID -> order
 *
 * */

#pragma once

#include "types.hpp"
#include "orderv2.hpp"
#include "trade.hpp"
#include "price_level.hpp"
#include "intrusive_listv2.hpp"
#include "ring_price_ladder.hpp"
#include "order_store.hpp"



// DTO: Data Transfer Obj
// transfer match result from orderbook to matching engine
struct MatchResult {

	bool	matched {};
	Trade   trade;
};



class Orderbook {

private:

    RingPriceLadder bids_;
    RingPriceLadder asks_;

    zerok::OrderStore order_store_;


public:

    Orderbook() : 
        bids_( 9000, Side::Bid ),
        asks_( 9000, Side::Ask ),
	order_store_() {}


    InternalID create_order ( OrderID external_id,
		              Price price,
			      Qty qty,
			      Side side );

    void add_order ( InternalID id );

    bool cancel_order ( InternalID id );    

    bool match_order ( MatchResult& result );

    bool modify_order ( InternalID id,
		    	Price new_price,
			Qty new_qty );

    Order& order (InternalID id ) noexcept;

    std::size_t size() const noexcept;
    
    void print_book() const noexcept;

};

