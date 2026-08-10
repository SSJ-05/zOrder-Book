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

	bool		matched {};
	Trade   	trade;
	Order*  	released [2] {};	// buy and/or sell - 2 orders/cases
	std::uint8_t	released_count {};	// counts cases to be released to pool
};



class Orderbook {

private:

    RingPriceLadder bids_;
    RingPriceLadder asks_;

    zerok::OrderStore order_store_ {};


public:

    Orderbook() : 
        bids_( 9000, Side::Bid ),
        asks_( 9000, Side::Ask ),
	order_store_() {}


    InternalID create_order ( /* */ );

    void add_order ( InternalID );

    bool match_order ( MatchResult& );

    bool cancel_order ( InternalID );    

    // copy order -> cancel_order -> change price,qty -> add_order
    Order*  modify_order ( OrderID, Price, Qty );

    [[ nodiscard ]]
    std::size_t size() const noexcept;
    
    void print_book() const noexcept;

};

