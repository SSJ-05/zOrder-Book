// order book header file// 07.07.26// ZeroK

#pragma once

#include "types.hpp"
#include "orderv2.hpp"
#include "trade.hpp"
#include "price_level.hpp"
#include "intrusive_listv2.hpp"

#include <map>
#include <unordered_map>


// helps in identifying Price Level to be modified
// w/o this, scan the whole order_map_
struct OrderLocation {

    PriceLevel*  level  { nullptr };
    Order*       order  { nullptr };
};



class Orderbook {

private:
    using BidMap = std::map <Price, PriceLevel, std::greater<>> ;  
    using AskMap = std::map <Price, PriceLevel> ;                  

    BidMap bids_;
    AskMap asks_;

    using BidIterator = BidMap::iterator;
    using AskIterator = AskMap::iterator;

    // order map to cancel/modify in O(1)
    std::unordered_map <OrderID, OrderLocation> order_map_;

    // helper funcs
    void erase_if_empty_bid (BidIterator);
    void erase_if_empty_ask (AskIterator);


public:
    void add_order (Order*);

    bool match_order (Trade&);

    // uo map lookup -> erase order -> erase hash entry
    bool cancel_order (OrderID);    

    // copy order -> cancel_order -> change price,qty -> add_order
    bool modify_order (OrderID, Price, Qty);

    void print_book () const noexcept;

};

