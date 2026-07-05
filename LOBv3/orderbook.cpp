// orderbook src file// 04.07.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "price_level.hpp"

#include <cstdio>
#include <algorithm>


void Orderbook::erase_if_empty (BidIterator it) {

    if (it->second.orders.empty()) 
        bids_.erase (it);
}

void Orderbook::erase_if_empty (AskIterator it) {

    if (it->second.orders.empty()) 
        asks_.erase (it);
}



void Orderbook::add_order (const Order& order) {

    if (order.side == Side::Buy) {
        auto [it, _] = 
            bids_.try_emplace (order.price, order.price);
        
        auto& orders = it->second.orders;

        orders.push_back (order);

        auto order_it = std::prev (orders.end());   // points to order

        it->second.total_qty += order.qty;

        order_map_.emplace (
            order.id, 
            OrderLocation {
                order.side,
                order.price,
                order_it }
        );
    }
    else {
        auto [it, _] = 
            asks_.try_emplace (order.price, order.price);
        
        auto& orders = it->second.orders;

        orders.push_back (order);

        auto order_it = std::prev (orders.end());

        it->second.total_qty += order.qty;

        order_map_.emplace (
            order.id, 
            OrderLocation {
                order.side,
                order.price,
                order_it }
        );
    }
}


/* lookup order
     * find side
     * find price level
     * decrement total_qty
     * erase iterator
     * erase empty level
     * erase hash entry
     * */
bool Orderbook::cancel_order (OrderID id) {

    // lookup order
    auto pos = order_map_.find (id);
    if (pos == order_map_.end()) 
        return false;

    OrderLocation location = pos->second;

    if (location.side == Side::Buy) {

        // lookup price level
        auto level = bids_.find (location.price);
        if (level == bids_.end()) return false;
        
        // decrement level qty
        level->second.total_qty -= location.it->qty;

        // erase order from price level
        level->second.orders.erase (location.it);

        // remove level if empty
        erase_if_empty (level);

        // erase hash entry
        order_map_.erase (pos);
    }
    else {      // Side::Sell

        auto level = asks_.find (location.price);
        
        level->second.total_qty -= location.it->qty;

        level->second.orders.erase (location.it);

        erase_if_empty (level);

        order_map_.erase (pos);
    }

    return true;
}



bool Orderbook::modify_order (OrderID id, 
                              Price new_price, 
                              Qty new_qty) {

    auto pos = order_map_.find (id);
    if (pos == order_map_.end()) 
        return false;

    OrderLocation location = pos->second;
    Order copy = *location.it;

    cancel_order (id);

    copy.price = new_price;
    copy.qty   = new_qty;

    add_order (copy);

    return true;
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
    // first erase from order_map then pop from BidMap
    // to avoid dangling refs
    if (bid.qty == 0) {
        order_map_.erase (bid.id);
        best_bid->second.orders.pop_front();
    }
    if (ask.qty == 0) {
        order_map_.erase (ask.id);
        best_ask->second.orders.pop_front();
    }

    // remove empty levels
    erase_if_empty (best_bid); 
    erase_if_empty (best_ask); 

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
                "SELL"
            );
        }

        std::printf("\n");
    }

    std::printf("======================================\n\n");
}
