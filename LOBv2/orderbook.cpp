// orderbook src file// 04.07.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "price_level.hpp"

#include <cstdio>
#include <algorithm>


void Orderbook::add_order (const Order& order) {

    if (order.side == Side::Buy) {
        auto [it, inserted] = 
            bids_.try_emplace (order.price, order.price);

        it->second.orders.push_back (order);
        it->second.total_qty += order.qty;
    }
    else {  // Side::Sell
        auto [it, inserted] = 
            asks_.try_emplace (order.price, order.price);

        it->second.orders.push_back (order);
        it->second.total_qty += order.qty;
    }

}



// review algo
// std::optional<Trade> Orderbook::match_order (Order& incoming) {
// bool Orderbook::match_order (Order& incoming, Trade& trade) {
bool Orderbook::match_order (Trade& trade) {

    auto best_bid = bids_.begin();
    auto best_ask = asks_.begin();

    if (bids_.empty()) return false;
    if (asks_.empty()) return false;

    if (best_bid->first < best_ask->first) return false;

    // update trade members
    auto& bid = best_bid->second.orders.front();
    auto& ask = best_ask->second.orders.front();

    trade.qty = std::min (bid.qty, ask.qty);
    bid.qty  -= trade.qty;
    ask.qty  -= trade.qty;

    // update LevelQty
    best_bid->second.total_qty -= trade.qty;
    best_ask->second.total_qty -= trade.qty;

    trade.buy_id  = bid.id;
    trade.sell_id = ask.id;

    trade.buy_price  = bid.price;
    trade.sell_price = ask.price;
    trade.exec_price = ask.price;

    // remove bid ask for matched orders
    if (bid.qty == 0) best_bid->second.orders.pop_front();
    if (ask.qty == 0) best_ask->second.orders.pop_front();

    // remove empty levels
    if (best_bid->second.orders.empty()) 
        bids_.erase (best_bid);

    if (best_ask->second.orders.empty()) 
        asks_.erase (best_ask);
    

    return true;
}


void Orderbook::print_book() const noexcept {

    std::printf("\n============= zORDER BOOK =============\n\n");

    if (bids_.empty())
        std::printf("Best Bid : EMPTY\n");
    else
        std::printf("Best Bid : %.2f\n", bids_.begin()->first / 100.0);

    if (asks_.empty())
        std::printf("Best Ask : EMPTY\n");
    else
        std::printf("Best Ask : %.2f\n", asks_.begin()->first / 100.0);

    if (!bids_.empty() && !asks_.empty()) {

        auto spread =
            (asks_.begin()->first - bids_.begin()->first) / 100.0;

        std::printf("Spread   : %.2f\n", spread);
    }
    else {
        std::printf("Spread   : N/A\n");
    }

    /**************************************************/

    std::printf("\n--------------- BIDS ----------------\n\n");

    for (const auto& [price, level] : bids_) {

        std::printf(
            "Price : %.2f    Total Qty : %u\n",
            price / 100.0,
            level.total_qty
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
                "BUY"
            );
        }

        std::printf("\n");
    }

    /**************************************************/

    std::printf("\n--------------- ASKS ----------------\n\n");

    for (const auto& [price, level] : asks_) {

        std::printf(
            "Price : %.2f    Total Qty : %u\n",
            price / 100.0,
            level.total_qty
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
                "SELL"
            );
        }

        std::printf("\n");
    }

    std::printf("======================================\n\n");
}
