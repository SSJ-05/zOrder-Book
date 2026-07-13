// ring price ladder src file// 08.07.26// ZeroK

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"
#include "ring_price_ladder.hpp"

#include <array>
#include <cstdint>


// helper func to calculate index
// logical idx  = price - base
// physical idx = logical & MASK
std::size_t RingPriceLadder::to_idx (Price p) const noexcept {
        
    return 
        static_cast<std::size_t>( p - base_price_ ) & MASK_;
}


bool RingPriceLadder::contains (Price p) const noexcept {
    
    return p >= base_price_
        && p <  base_price_ + static_cast<Price>( NUM_LEVELS_ );
}


// getter funcs for rpl_[index]
PriceLevel&  RingPriceLadder::at_level (Price p) noexcept {

    assert( contains ( p ) );
    return rpl_[to_idx( p )];
}



void RingPriceLadder::add (Order* order) noexcept {
    
    if ( !contains( order->price ) ) 
        advance_window( order->price );

    else {
        PriceLevel& level = at_level( order->price );
        
        if ( level.price != order->price ) {
            level.price      =  order->price;
            level.total_qty  =  0;
        }

        level.orders.push_back( order );

        level.total_qty += order->qty;

        update_best_after_add( order->price );
    }
}



void RingPriceLadder::update_best_after_add (Price new_price) noexcept {
    
    const std::size_t idx = to_idx( new_price );
    
    // first order in orderbook
    if ( at_level( new_price ).orders.size() == 1 ) {
        best_idx_ = idx;
        return;
    }

    Price best_price = base_price_ 
                       + static_cast<Price>( best_idx_ );

    if ( side_ == Side::Bid ) {
        if ( new_price > best_price_ ) 
            best_idx_ = idx;
    }
    else {   // Ask
        if ( new_price < best_price_ )
            best_idx_ = idx;
    }
}



void RingPriceLadder::update_best_after_remove (Price removed_price) noexcept {

    if ( to_idx( removed_price ) != best_idx_ ) return;

    // bid
    if (side_ == Side::Bid) {
        for (auto idx {best_idx_}; idx-- > 0;) {
            if ( !rpl_[idx].orders.empty() ) {
                best_idx_ = idx;
                return;
            }
        }
    }

    // ask
    else {
        for (auto idx {best_idx_}; idx < NUM_LEVELS_; ++idx) {
            if ( !rpl_[idx].orders.empty() ) {
                best_idx_ = idx;
                return;
            }
        }
    }

}



void RingPriceLadder::remove (Order* order) noexcept {
    
    assert( order != nullptr );

    PriceLevel& level = at_level( order->price );

    // remove from FIFO
    level.orders.erase( order );

    // update total qty
    level.total_qty -= order->qty;

    // check if level still has orders
    if ( !level.orders.empty() ) return;

    // 1. if level becomes empty after remove
    update_best_after_remove( order->price );

    // 2. and reset the level
    level.total_qty = 0;

}


void  RingPriceLadder::clear_level (Price new_price) noexcept {

    PriceLevel& lvl    =  at_level( new_price );
    lvl.price          =  new_price;
    lvl.total_qty      =  0;

    level.orders.clear();
}


PriceLevel* RingPriceLadder::best_level() noexcept {

    return rpl_[best_idx_];
}


void RingPriceLadder::advance_window (Price new_base) {

    assert( false && "NOTE: Not implemented yet.\n" );
} 
