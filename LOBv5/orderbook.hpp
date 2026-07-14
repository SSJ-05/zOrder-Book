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

    bool match_order (Trade&);

    // uo map lookup -> erase order -> erase hash entry
    bool cancel_order (OrderID);    

    // copy order -> cancel_order -> change price,qty -> add_order
    bool modify_order (OrderID, Price, Qty);

    void print_book() const noexcept;

};

