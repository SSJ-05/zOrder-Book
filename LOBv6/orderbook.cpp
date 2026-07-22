// orderbook src file// 07.07.26// ZeroK

#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "price_level.hpp"
#include "intrusive_listv2.hpp"

#include <cstdio>
#include <algorithm>



void Orderbook::add_order (Order* order) {

    if ( order->side == Side::Bid )
        bids_.add( order );
    else 
        asks_.add( order );
        

    order_map_.emplace (
            order->id, 
            OrderLocation { order }
        );
}


Order*  Orderbook::cancel_order (OrderID id) {

    // lookup order in hashmap
    auto pos = order_map_.find( id );
    if ( pos == order_map_.end() ) 
        return false;

    Order* order = pos->second.order;   // cache the ptr

    if ( order->side == Side::Bid )
        bids_.remove( order );
    else
        asks_.remove( order );

    order_map_.erase( pos );    // erase hash entry O(1)

    return order;
}



Order*  Orderbook::modify_order ( OrderID id, 
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

    return order;
}



bool Orderbook::match_order ( MatchResult& result ) {

    PriceLevel* bid_lvl = bids_.best_level();
    PriceLevel* ask_lvl = asks_.best_level();

    if ( bid_lvl == nullptr || ask_lvl == nullptr )
        return false;

    if ( bid_lvl->price < ask_lvl->price ) 
        return false;

    Order* bid = bid_lvl->orders.front();
    Order* ask = ask_lvl->orders.front();

    // update trade members
    trade.qty = std::min (bid->qty, ask->qty);
    bid->qty  -= trade.qty;
    ask->qty  -= trade.qty;

    // update LevelQty and price
    bid_lvl->total_qty -= trade.qty;
    ask_lvl->total_qty -= trade.qty;

    trade.buy_id  = bid->id;
    trade.sell_id = ask->id;

    trade.buy_price  = bid_lvl->price;
    trade.sell_price = ask_lvl->price;
    trade.exec_price = ask_lvl->price;


    // order obj leaves the orderbook at this
    // remove bid ask for matched orders
    if ( bid->qty == 0 ) {
        order_map_.erase( bid->id );
        bids_.remove( bid );

	result.released [ result.released_count++ ] = bid;  // set the status for release
    }
    if ( ask->qty == 0 ) {
        order_map_.erase( ask->id );
        asks_.remove( ask );

	result.released [ result.released_count++ ] = ask;
    }

    return true;
}



// void Orderbook::print_book() const noexcept {
//
//     std::printf("\n============= zORDER BOOK =============\n\n");
//
//     if (bids_.empty())
//         std::printf("Best Bid : EMPTY\n");
//     else
//         std::printf("Best Bid : %.2f\n", 
//                     to_price (bids_.begin()->first));
//
//     if (asks_.empty())
//         std::printf("Best Ask : EMPTY\n");
//     else
//         std::printf("Best Ask : %.2f\n", 
//                     to_price (asks_.begin()->first));
//
//     if (!bids_.empty() && !asks_.empty()) {
//
//         auto spread =
//             (asks_.begin()->first - bids_.begin()->first);
//
//         std::printf("Spread   : %.2f\n", to_price (spread));
//     }
//     else {
//         std::printf("Spread   : N/A\n");
//     }
//
//     /**************************************************/
//
//     std::printf("\n--------------- BIDS ----------------\n\n");
//
//     for (const auto& [price, level] : bids_) {
//
//         std::printf(
//             "Price : %.2f    Total Qty : %u     Orders : %zu\n" ,
//             to_price (price), level.total_qty, level.orders.size()
//         );
//
//         std::printf(
//             "%-10s %-10s %-10s\n",
//             "ID",
//             "QTY",
//             "SIDE"
//         );
//
//         for ( Order* p = level.orders.front();
//               p; 
//               p = level.orders.next( p ) ) {
//
//             std::printf(
//                 "%-10llu %-10u %-10s\n",
//                 static_cast<unsigned long long>(p->id),
//                 p->qty,
//                 "BID"
//             );
//         }
//
//         std::printf("\n");
//     }
//
//     /**************************************************/
//
//     std::printf("\n--------------- ASKS ----------------\n\n");
//
//     for (const auto& [price, level] : asks_) {
//
//         std::printf(
//             "Price : %.2f    Total Qty : %u     Orders : %zu\n" ,
//             to_price (price), level.total_qty, level.orders.size()
//         );
//
//         std::printf(
//             "%-10s %-10s %-10s\n",
//             "ID",
//             "QTY",
//             "SIDE"
//         );
//
//         for ( Order* p = level.orders.front();
//               p; 
//               p = level.orders.next( p ) ) {
//
//             std::printf(
//                 "%-10llu %-10u %-10s\n",
//                 static_cast<unsigned long long>(p->id),
//                 p->qty,
//                 "ASK"
//             );
//         }
//
//         std::printf("\n");
//     }
//
//     std::printf("======================================\n\n");
// }

