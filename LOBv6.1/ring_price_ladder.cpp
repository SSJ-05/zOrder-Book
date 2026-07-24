// ring price ladder src file// 08.07.26// ZeroK

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"
#include "ring_price_ladder.hpp"

#include <cassert>


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



// for read-only in print_book
const PriceLevel&  RingPriceLadder::at_level (Price p) const noexcept {

    assert( contains ( p ) );
    return rpl_[to_idx( p )];
}



void RingPriceLadder::add (Order* order) noexcept {
    
    if ( !contains( order->price ) ) return;
        // advance_window( order->price );

    PriceLevel& lvl = at_level( order->price );
        
    if ( lvl.price != order->price ) {
        lvl.price      =  order->price;
        lvl.total_qty  =  0;
    }

    lvl.orders.push_back( order );

    lvl.total_qty += order->qty;

    update_best_after_add( order->price );

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
        if ( new_price > best_price ) 
            best_idx_ = idx;
    }
    else {   // Ask
        if ( new_price < best_price )
            best_idx_ = idx;
    }
}



void RingPriceLadder::update_best_after_remove (Price removed_price) noexcept {

    if ( to_idx( removed_price ) != best_idx_ ) return;

    // bid
    if (side_ == Side::Bid) {
        for (auto idx {0uz}; idx < best_idx_; ++idx) {
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

    best_idx_ = INVALID_;

}



void RingPriceLadder::remove (Order* order) noexcept {
    
    assert( order != nullptr );

    PriceLevel& lvl = at_level( order->price );

    // remove from FIFO
    lvl.orders.erase( order );

    // update total qty
    lvl.total_qty -= order->qty;

    // check if level still has orders
    if ( !lvl.orders.empty() ) return;

    // 1. if level becomes empty after remove
    update_best_after_remove( order->price );

    // 2. and reset the level
    lvl.total_qty = 0;

}



void  RingPriceLadder::clear_level (Price new_price) noexcept {

    PriceLevel& lvl    =  at_level( new_price );
    lvl.price          =  new_price;
    lvl.total_qty      =  0;

    lvl.orders.clear();
}



PriceLevel* RingPriceLadder::best_level() noexcept {

    if ( best_idx_ == INVALID_ ) return nullptr;

    return &rpl_[best_idx_];
}

const PriceLevel* RingPriceLadder::best_level() const noexcept {

    if ( best_idx_ == INVALID_ ) return nullptr;

    return &rpl_[best_idx_];
}



// void RingPriceLadder::advance_window ([[ maybe_unused ]]Price new_base) {
//
//     assert( false && "NOTE: Not implemented yet.\n" );
// } 
