// orderbook src file// 07.07.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "price_level.hpp"
#include "intrusive_list.hpp"

#include <cstdio>
#include <algorithm>


void Orderbook::erase_if_empty_bid (BidIterator it) {

    if (it->second.orders.empty()) 
        bids_.erase (it);
}

void Orderbook::erase_if_empty_ask (AskIterator it) {

    if (it->second.orders.empty()) 
        asks_.erase (it);
}



void Orderbook::add_order (Order* order) {

    PriceLevel* level;

    if (order->side == Side::Bid) {
        level = &bids_.try_emplace (
                order->price, order->price).first->second;
    }
    else {
        level = &asks_.try_emplace (
                order->price, order->price).first->second;
    }
        
    level->orders.push_back( order );

    level->total_qty += order->qty;

    order_map_.emplace (
            order->id, 
            OrderLocation { level, order }
        );
}


bool Orderbook::cancel_order (OrderID id) {

    // lookup order in hashmap
    auto pos = order_map_.find( id );
    if ( pos == order_map_.end() ) 
        return false;

    PriceLevel* level = pos->second.level;  // cache level
    Order* order = pos->second.order;       // cache the ptr

    level->total_qty -= order->qty;     // dec qty
    level->orders.erase( order );       // erase from intrusivelist O(1)


    if ( level->orders.empty() ) {      // erase empty levels (log N)

        if ( order->side == Side::Bid )
            bids_.erase( level->price );
        else
            asks_.erase( level->price );
    }

    order_map_.erase( pos );    // erase hash entry O(1)

    return true;
}



bool Orderbook::modify_order ( OrderID id, 
                               Price new_price, 
                               Qty new_qty ) {

    auto pos = order_map_.find( id );
    if ( pos == order_map_.end() ) 
        return false;

    Order* order = pos->second.order;

    cancel_order( id );

    order->price = new_price;
    order->qty   = new_qty;

    add_order( order );

    return true;
}



bool Orderbook::match_order ( Trade& trade ) {

    if (bids_.empty()) return false;
    if (asks_.empty()) return false;

    auto best_bid = bids_.begin();
    auto best_ask = asks_.begin();

    if (best_bid->first < best_ask->first) return false;

    // cache levels
    PriceLevel* bid_level = &best_bid->second;   
    PriceLevel* ask_level = &best_ask->second;

    Order* bid = bid_level->orders.front();
    Order* ask = ask_level->orders.front();

    // update trade members
    trade.qty = std::min (bid->qty, ask->qty);
    bid->qty  -= trade.qty;
    ask->qty  -= trade.qty;

    // update LevelQty and price
    bid_level->total_qty -= trade.qty;
    ask_level->total_qty -= trade.qty;

    trade.buy_id  = bid->id;
    trade.sell_id = ask->id;

    trade.buy_price  = best_bid->first;
    trade.sell_price = best_ask->first;
    trade.exec_price = best_ask->first;

    // remove bid ask for matched orders
    // first erase from order_map then pop from BidMap
    // to avoid dangling refs
    if ( bid->qty == 0 ) {
        order_map_.erase( bid->id );
        bid_level->orders.erase( bid );
    }
    if ( ask->qty == 0 ) {
        order_map_.erase( ask->id );
        ask_level->orders.erase( ask );
    }

    // remove empty levels
    erase_if_empty_bid( best_bid ); 
    erase_if_empty_ask( best_ask ); 

    return true;
}


void Orderbook::print_book() const noexcept {

    std::printf("\n============= zORDER BOOK =============\n\n");

    if (bids_.empty())
        std::printf("Best Bid : EMPTY\n");
    else
        std::printf("Best Bid : %.2f\n", 
                    to_price (bids_.begin()->first));

    if (asks_.empty())
        std::printf("Best Ask : EMPTY\n");
    else
        std::printf("Best Ask : %.2f\n", 
                    to_price (asks_.begin()->first));

    if (!bids_.empty() && !asks_.empty()) {

        auto spread =
            (asks_.begin()->first - bids_.begin()->first);

        std::printf("Spread   : %.2f\n", to_price (spread));
    }
    else {
        std::printf("Spread   : N/A\n");
    }

    /**************************************************/

    std::printf("\n--------------- BIDS ----------------\n\n");

    for (const auto& [price, level] : bids_) {

        std::printf(
            "Price : %.2f    Total Qty : %u\n",
            to_price (price), level.total_qty
        );

        std::printf(
            "%-10s %-10s %-10s\n",
            "ID",
            "QTY",
            "SIDE"
        );

        for (const auto& order : level.orders) {

            std::printf(
                "%-10llu %-10u %-10s\n",
                static_cast<unsigned long long>(order.id),
                order.qty,
                "BID"
            );
        }

        std::printf("\n");
    }

    /**************************************************/

    std::printf("\n--------------- ASKS ----------------\n\n");

    for (const auto& [price, level] : asks_) {

        std::printf(
            "Price : %.2f    Total Qty : %u\n",
            to_price (price), level.total_qty
        );

        std::printf(
            "%-10s %-10s %-10s\n",
            "ID",
            "QTY",
            "SIDE"
        );

        for (const auto& order : level.orders) {

            std::printf(
                "%-10llu %-10u %-10s\n",
                static_cast<unsigned long long>(order.id),
                order.qty,
                "ASK"
            );
        }

        std::printf("\n");
    }

    std::printf("======================================\n\n");
}
