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
        center_window( order->price );
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
	center_window( new_price );
        return;
    }

    Price best_price = base_price_ + static_cast<Price>( best_idx_ );
    // Price best_price  =  rpl_[ best_idx_ ].price;


    if ( side_ == Side::Bid ) {
        if ( new_price > best_price ) 
            best_idx_ = to_idx( new_price );
	    center_window( new_price );
    }
    else {   // Ask
        if ( new_price < best_price )
            best_idx_ = to_idx( new_price );
	    center_window( new_price );
    }

}



void  RingPriceLadder::update_best_after_remove (Price removed_price) noexcept {

    if ( to_idx( removed_price ) != best_idx_ ) return;


    if (side_ == Side::Bid) {

	    // Phase 1: scan downward in window
	    for ( Price p = best_price - 1; p >= window_low_; --p ) {
		
		    if ( !at_level( p ).orders.empty() ) {
			    best_idx_  =  to_idx( p );
#ifndef NDEBUG
			    ++window_hits_;
#endif
			    return;
		    }
	    }

	    // Phase 2: fallback - scan outside window (below window_low_)
	    while ( advance_window_up() ) {
		
		    for ( Price search_p = window_high_;
			  search_p > window_low_; --search_p ) {
			
			    if ( !at_level( search_p ).orders.empty() ) {
				    best_idx_  =  to_idx( search_p );
#ifndef NDEBUG
				    ++window_misses_;
#endif
				    return;
			    }
		    }
	    }

	    // Phase 3: fallback - full ring scan if window cant slide
	    for ( auto i {1uz}; i < NUM_LEVELS_; ++i ) {
		
		    std::size_t idx = (best_idx_ - i) & MASK_;
		    
		    if ( !rpl_[ idx ].orders.empty() ) {
			    best_idx_  =  idx;
#ifndef NDEBUG
			    ++window_misses_;
#endif
			    return;
		    }
	    }
    }
    else {	// ask
	    // Phase 1: scan window
	    for (Price p = best_price + 1; p <= window_high_; ++p) {
		
		    if ( !at_level( p ).orders.empty() ) {
			    best_idx_  =  to_idx( p );
#ifndef NDEBUG
			    ++window_hits_;
#endif
			    return;
		    }
	    }

	    // Phase 2: fallback - search outside window (above window_high)
	    while ( advance_window_down() ) {

		    for ( Price search_p = window_low_;
			  search_p < window_high_; ++search_p ) {

			    if ( !at_level( search_p ).orders.empty() ) {
				    best_idx_  =  to_idx( search_p );
#ifndef NDEBUG
				    ++window_misses_;
#endif
				    return;
			    }
		    }
	    }

	    // Phase 3: fallback - full ring scan
	    for ( auto i {1uz}; i < NUM_LEVELS_; ++i ) {
		
		    std::size_t idx = (best_idx_ + i) & MASK_;
		    
		    if ( !rpl_[ idx ].orders.empty() ) {
			    best_idx_  =  idx;
#ifndef NDEBUG
			    ++window_misses_;
#endif
			    return;
		    }
	    }
    }	// else

    // not found in window
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



// sliding window ops
[[ nodiscard ]]
bool  RingPriceLadder::in_window ( Price p ) const noexcept {
	
	return  p >= window_low_ &&
		p <= window_high_;
}


void  RingPriceLadder::center_window ( Price center ) noexcept {

	Price ring_max  =  base_price_ + static_cast<Price>( NUM_LEVELS_ - 1 );

	Price low   =  center - static_cast<Price>( WINDOW_SIZE_ >> 1 );
	Price high  =  center + static_cast<Price>( WINDOW_SIZE_ >> 1 );

	// maintain the size of window
	if ( low < base_price_ ) low   ==  base_price_;
	if ( high > ring_max )   high  ==  ring_max;


	window_low_  =  low;
	window_high_ =  high;
}


