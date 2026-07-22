// order book header file// 07.07.26// ZeroK

#pragma once

#include "types.hpp"
#include "orderv2.hpp"
#include "trade.hpp"
#include "price_level.hpp"
#include "intrusive_listv2.hpp"
#include "ring_price_ladder.hpp"

#include <unordered_map>


// helps in identifying Price Level to be modified
// w/o this, scan the whole order_map_
struct OrderLocation {

    Order*  order  { nullptr };
};



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

    // order map to cancel/modify in O(1)
    std::unordered_map <OrderID, OrderLocation> order_map_;
    // ***benchmark before replacing std::unordered_map with flat_map


public:

    Orderbook() : 
        bids_( 9000, Side::Bid ),
        asks_( 9000, Side::Ask ) {}


    void add_order (Order*);

    bool match_order (MatchResult&);

    // uo map lookup -> erase order -> erase hash entry
    Order*  cancel_order (OrderID);    

    // copy order -> cancel_order -> change price,qty -> add_order
    Order*  modify_order (OrderID, Price, Qty);

    void print_book() const noexcept;

};

