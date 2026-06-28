// orderbook src file// 27.06.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"

#include <cstdio>
#include <vector>
#include <algorithm>    // std::max, min


void Orderbook::add_order (const Order& order) {

    if (order.side == BUY) {
        bids_.push_back (order);
        if (order.price >= best_bid) best_bid = order.price;
    }

    else {
        asks_.push_back (order);
        if (order.price <= best_ask) best_ask = order.price;
    }
}



// review algo
// std::optional<Trade> Orderbook::match_order (Order& incoming) {
// bool Orderbook::match_order (Order& incoming, Trade& trade) {
bool Orderbook::match_order (Trade& trade) {

    // cal max bid
    auto max_bid = std::max_element (
           bids_.cbegin(), bids_.cend(),
           [] (const Order& a, const Order& b) {
                return a.price < b.price;
           } );
    
    // cal min ask
    auto min_ask = std::min_element (
           asks_.cbegin(), asks_.cend(),
           [] (const Order& a, const Order& b) {
                return a.price < b.price;
           } );


    if (max_bid == bids_.end()) return false;
    if (min_ask == asks_.end()) return false;
    if (max_bid->price < min_ask->price) return false;

    // update trade qty
    trade.qty = std::min (max_bid->qty,
                          min_ask->qty);

    trade.price = min_ask->price;

    // update id
    trade.buy_id  = max_bid->id;
    trade.sell_id = min_ask->id;

    // update qty of bid and ask
    max_bid->qty -= trade.qty;
    min_ask->qty -= trade.qty;

    // remove bid ask if matched
    if (max_bid->qty == 0) bids_.erase (max_bid);
    if (min_ask->qty == 0) asks_.erase (min_ask);


    return true;
}



// debuggin only
// void Orderbook::print_book () const noexcept {
//
//     std::printf ("\n\n.......BIDS.........\n");
//     for (auto i {0}; i < 5; ++i) std::printf (bids_[i]);
//
//     std::printf ("\n\n.......ASKS.........\n");
//     for (auto i {0}; i < 5; ++i) std::printf (bids_[i]);
//
//     std::printf ("best bid = %.2f\n"
//                  "best ask = %.2f\n"
//                  "spread   = %.2f\n",
//
//
// }

