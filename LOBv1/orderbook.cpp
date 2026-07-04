// orderbook src file// 27.06.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"

#include <cstdio>
#include <vector>
#include <algorithm>    // std::max, min


void Orderbook::add_order (const Order& order) {

    if (order.side == Side::Buy) {
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

    auto max_bid = std::max_element (
           bids_.begin(), bids_.end(),
           [] (const Order& a, const Order& b) {
                return a.price < b.price;
           } ); 

    auto min_ask = std::min_element (
           asks_.begin(), asks_.end(),
           [] (const Order& a, const Order& b) {
                return a.price < b.price;
           } ); 


    if (max_bid == bids_.end()) { 
        best_bid = std::numeric_limits<double>::lowest();
        return false;
    }
    
    if (min_ask == asks_.end()) {
        best_ask = std::numeric_limits<double>::max();
        return false;
    }

    if (max_bid->price < min_ask->price) return false;


    best_bid = max_bid->price;
    best_ask = min_ask->price;

    // update trade members
    trade.qty = std::min (max_bid->qty,
                          min_ask->qty);

    trade.buy_id  = max_bid->id;
    trade.sell_id = min_ask->id;

    trade.buy_price  = max_bid->price;
    trade.sell_price = min_ask->price;

    trade.exec_price = min_ask->price;


    // update qty of bid and ask
    max_bid->qty -= trade.qty;
    min_ask->qty -= trade.qty;

    // remove bid ask if matched
    if (max_bid->qty == 0) bids_.erase (max_bid);
    if (min_ask->qty == 0) asks_.erase (min_ask);


    return true;
}



void Orderbook::print_book () const noexcept {

    std::printf("\n================ ORDER BOOK ================\n\n");
std::printf(
    "NEW %-4s id=%3lu px=%6.2f qty=%3u\n",
    order.side == Side::Buy ? "BUY" : "SELL",
    order.id,
    order.price,
    order.qty
);
    if (bids_.empty()) std::printf ("Best Bid : EMPTY\n");
    else std::printf("Best Bid : %.2f\n", best_bid);

    if (asks_.empty()) std::printf ("Best Ask : EMPTY\n");
    else std::printf("Best Ask : %.2f\n", best_ask);

    if (!bids_.empty() && !asks_.empty())
        std::printf("Spread   : %.2f\n\n", best_ask - best_bid);
    else 
        std::printf ("Spread   : N/A\n\n");

    std::printf("--------------- BIDS ----------------\n");
    std::printf("%-10s %-10s %-10s\n", "ID", "PRICE", "QTY");

    for (const auto& order : bids_) {
        std::printf("%-10llu %-10.2f %-10u\n",
                static_cast<unsigned long long>(order.id),
                order.price,
                order.qty);
    }

    std::printf("\n--------------- ASKS ----------------\n");
    std::printf("%-10s %-10s %-10s\n", "ID", "PRICE", "QTY");

    for (const auto& order : asks_) {
        std::printf("%-10llu %-10.2f %-10u\n",
                static_cast<unsigned long long>(order.id),
                order.price,
                order.qty);
    }

    std::printf("\n============================================\n");
}
