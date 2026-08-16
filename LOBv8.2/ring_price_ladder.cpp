// ring price ladder src file// 13.08.26// ZeroK

/* Changes:
 * current - remove best -> scan 8192 levels
 * proposed - remove best -> if found in active window, scan 2048 levels
 *			  -> else fallback to scan 8192 levels
 * */

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"
#include "ring_price_ladder.hpp"

#include <cassert>
#include <cstdio>
#include <limits>


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
    return rpl_[ to_idx( p ) ];
}



// for read-only in print_book
const PriceLevel&  RingPriceLadder::at_level (Price p) const noexcept {

    assert( contains ( p ) );
    return rpl_[ to_idx( p ) ];
}



void RingPriceLadder::add (Order* order) noexcept {
    
    if ( !contains( order->price ) ) {
	std::printf( "OUT OF RANGE : %u\n", order->price );
	assert( false );
        // advance_window( order->price );
    }

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
    
    if ( best_idx_ == INVALID_ ) {
        best_idx_ = to_idx( new_price );
        return;
    }

    // Price best_price = base_price_ 
    //                    + static_cast<Price>( best_idx_ );
    Price best_price  =  rpl_[ best_idx_ ].price;


    if ( side_ == Side::Bid ) {
        if ( new_price > best_price ) 
            best_idx_ = to_idx( new_price );
    }
    else {   // Ask
        if ( new_price < best_price )
            best_idx_ = to_idx( new_price );
    }

}



void  RingPriceLadder::update_best_after_remove (Price removed_price) noexcept {

    if ( to_idx( removed_price ) != best_idx_ ) return;

    if (side_ == Side::Bid) {

	    for (auto i {1uz}; i < NUM_LEVELS_; ++i) {
		
		    // scan backward from best_idx -1
		    std::size_t idx = (best_idx_ - i) & MASK_;

		    if ( !rpl_[ idx ].orders.empty() ) {
			    best_idx_  =  idx;
			    return;
		    }
	    }
    }
    else {	// ask

	    for (auto i {1uz}; i < NUM_LEVELS_; ++i) {
		
		    std::size_t idx = (best_idx_ + i) & MASK_;

		    if ( !rpl_[ idx ].orders.empty() ) {
			    best_idx_  =  idx;
			    return;
		    }
	    }
    }

    best_idx_  =  INVALID_;

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

    assert( lvl.orders.empty() );
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
