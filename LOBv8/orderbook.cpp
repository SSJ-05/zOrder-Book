// orderbook src file// 10.08.26// ZeroK
/* 
 * OrderBook
 ├── creates Order
 ├── adds Order
 ├── removes Order
 ├── releases Order
 └── owns OrderStore
 */


#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "price_level.hpp"
#include "intrusive_listv2.hpp"
#include "order_store.hpp"

#include <cstdio>
#include <algorithm>
#include <cassert>



Order&  Orderbook::order ( InternalID id ) noexcept {

	return order_store_[ id ];
}



InternalID Orderbook::create_order ( OrderID external_id,
				     Price price, 
				     Qty qty,
				     Side side ) {

	InternalID id  =  order_store_.acquire();

	if ( id == zerok::OrderStore::INVALID_ID )
		return id;

	Order& order  =  order_store_[ id ];

	order.external_id   =   external_id;
	order.internal_id   =   id;
	order.price         =   price;
	order.qty           =   qty;
	order.side	    =   side;

	add_order( id );

	return id;
}


void Orderbook::add_order ( InternalID id ) {

	Order& order  =  order_store_[ id ];

    	if ( order.side == Side::Bid )
        	bids_.add( &order );
    	else 
        	asks_.add( &order );
}



bool  Orderbook::cancel_order ( InternalID id ) {

    Order& order  =  order_store_[ id ];   

    if ( !order.inlist ) return false;	// already removed

    if ( order.side == Side::Bid )
        bids_.remove( &order );
    else
        asks_.remove( &order );

    order_store_.release( id );

    return true;
}



bool  Orderbook::modify_order ( InternalID id, 
                                Price new_price, 
                        	Qty new_qty ) {

    Order& order = order_store_[ id ];

    if ( !order.inlist ) return false;

    // remove
    if ( order.side == Side::Bid )
	    bids_.remove( &order );
    else
	    asks_.remove( &order );

    // modify
    order.price = new_price;
    order.qty   = new_qty;

    // reinsert
    add_order( id );

    return true;
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
    result.trade.qty = std::min (bid->qty, ask->qty);

    bid->qty  -= result.trade.qty;
    ask->qty  -= result.trade.qty;

    // update LevelQty and price
    bid_lvl->total_qty -= result.trade.qty;
    ask_lvl->total_qty -= result.trade.qty;

    result.trade.buy_id  = bid->external_id;
    result.trade.sell_id = ask->external_id;

    result.trade.buy_price  = bid_lvl->price;
    result.trade.sell_price = ask_lvl->price;
    result.trade.exec_price = ask_lvl->price;


    // order obj leaves the orderbook at this
    // remove bid ask for matched orders
    if ( bid->qty == 0 ) {

        bids_.remove( bid );

	order_store_.release( bid->internal_id );
    }


    if ( ask->qty == 0 ) {

        asks_.remove( ask );

	order_store_.release( ask->internal_id );
    }

    result.matched  =  true;

    return true;
}



std::size_t Orderbook::size() const noexcept {
	
	return order_store_.active_count();
}



void Orderbook::print_book() const noexcept {

    std::printf(
        "\n================== zORDER BOOK ==================\n\n");

    const PriceLevel* bid = bids_.best_level();
    const PriceLevel* ask = asks_.best_level();

    if ( !bid )
        std::printf("Best Bid : EMPTY\n");
    else
        std::printf("Best Bid : %.2f\n",
                    to_price(bid->price));

    if ( !ask )
        std::printf("Best Ask : EMPTY\n");
    else
        std::printf("Best Ask : %.2f\n",
                    to_price(ask->price));

    if ( bid && ask )
        std::printf("Spread   : %.2f\n",
                     to_price(ask->price - bid->price));
    else
        std::printf("Spread   : N/A\n");

    std::printf("\n=================================================\n");

std::printf("\n--------------- BIDS ----------------\n\n");

bids_.for_each_level([&](Price price, const PriceLevel& level)
{
    std::printf(
        "Price : %.2f    Total Qty : %u     Orders : %zu\n",
        to_price(price),
        level.total_qty,
        level.orders.size());

    std::printf(
        "%-10s %-10s %-10s %-10s\n",
        "EXT_ID",
        "INT_ID",
        "QTY",
        "SIDE");

    for (Order* p = level.orders.front();
         p;
         p = level.orders.next(p))
    {
        std::printf(
            "%-10llu %-10llu %-10u %-10s\n",
            static_cast<unsigned long long>(p->external_id),
            static_cast<unsigned long long>(p->internal_id),
            p->qty,
            "BID");
    }

    std::printf("\n");
});


std::printf("\n--------------- ASKS ----------------\n\n");

asks_.for_each_level([&](Price price, const PriceLevel& level)
{
    std::printf(
        "Price : %.2f    Total Qty : %u     Orders : %zu\n",
        to_price(price),
        level.total_qty,
        level.orders.size());

    std::printf(
        "%-10s %-10s %-10s %-10s\n",
        "EXT_ID",
        "INT_ID",
        "QTY",
        "SIDE");

    for (Order* p = level.orders.front();
         p;
         p = level.orders.next(p))
    {
        std::printf(
            "%-10llu %-10llu %-10u %-10s\n",
            static_cast<unsigned long long>(p->external_id),
            static_cast<unsigned long long>(p->internal_id),
            p->qty,
            "ASK");
    }

    std::printf("\n");
});

    std::printf(
        "=================================================\n\n");
}

