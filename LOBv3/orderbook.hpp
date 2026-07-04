// order book header file// 04.07.26// ZeroK

#pragma once

#include "types.hpp"
#include "order.hpp"
#include "trade.hpp"
#include "price_level.hpp"

#include <map>
#include <unordered_map>
#include <list>


using OrderIterator = std::list<Order>::iterator;


// helps in identifying Price Level to be modified
// w/o this, scan the whole order_map_
struct OrderLocation {

    Side           side;
    Price          price;
    OrderIterator  it;
};



class Orderbook {

private:
    using BidMap = std::map <Price, PriceLevel, std::greater<>> ;  
    using AskMap = std::map <Price, PriceLevel> ;                  

    using BidIterator = BidMap::iterator;
    using AskIterator = AskMap::iterator;

    BidMap bids_;
    AskMap asks_;

    // order map to cancel/modify in O(1)
    std::unordered_map <OrderID, OrderLocation> order_map_;

    // helper funcs
    void erase_level_if_empty (BidIterator);
    void erase_level_if_empty (AskIterator);


public:
    void add_order (const Order&);

    bool match_order (Trade&);

    // uo map lookup -> erase order -> erase hash entry
    bool cancel_order (OrderID);    

    // copy order -> cancel_order -> change price,qty -> add_order
    bool modify_order (OrderID, Price, Qty);

    void print_book () const noexcept;

};

