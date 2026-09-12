// ring price ladder src file// 13.08.26// ZeroK

/* Changes:
 * current - remove best -> scan 8192 levels
 * proposed - remove best -> if found in active window, scan 2048 levels
 *			  -> else fallback to scan 8192 levels
 * */

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"
#include "hot_price_level.hpp"

#include <cassert>
#include <cstdio>
#include <limits>


// helper func to calculate index
// logical idx  = price - base
// physical idx = logical & MASK
std::size_t HotPriceLevel::to_idx (Price p) const noexcept {
        
    return  static_cast<std::size_t>( p - base_price_ ) 
	    & MASK_;
}


bool HotPriceLevel::contains (Price p) const noexcept {
    
    return p >= base_price_
        && p <  base_price_ + static_cast<Price>( NUM_LEVELS_ );
}



void HotPriceLevel::update_best_after_add (Price new_price) noexcept {
    
    if ( best_idx_ == INVALID_ ) {
        best_idx_ = to_idx( new_price );
        return;
    }

    PriceLevel* best  =  hot_[ best_idx_ ];
    assert( best != nullptr );	

    const Price best_price  =  best->price;


    if ( side_ == Side::Bid ) {
        if ( new_price > best_price ) 
            best_idx_ = to_idx( new_price );
    }
    else {   // Ask
        if ( new_price < best_price )
            best_idx_ = to_idx( new_price );
    }
}



void  HotPriceLevel::update_best_after_remove (Price removed_price) noexcept {

    if ( best_idx_ == INVALID_ ) return;
    if ( to_idx( removed_price ) != best_idx_ ) return;

    /* demote() clears hot_[idx] before calling this
     * if removed price was best, hot[best_idx_] is now nullptr
     * */

    if ( side_ == Side::Bid ) {

	    for ( Price p {removed_price -1}; p >= window_low_; --p ) {
		
		    PriceLevel* level  =  find( p );

		    if ( level != nullptr ) {
			    best_idx_  =  to_idx( p );
#ifndef NDEBUG
			    ++window_hits_;
#endif
			    return;
		    }
	    }
    }
    else {	// ask

	    for ( Price p {removed_price +1}; p <= window_high_; ++p ) {
		
		    PriceLevel* level  =  find( p );
		    
		    if ( level != nullptr ) {
			    best_idx_  =  to_idx( p );
#ifndef NDEBUG
			    ++window_hits_;
#endif
			    return;
		    }
	    }
    }

    best_idx_  =  INVALID_;	// no levels in hot window

#ifndef NDEBUG
    ++window_misses_;
#endif
}



PriceLevel*  HotPriceLevel::best_level() noexcept {

    if ( best_idx_ == INVALID_ ) return nullptr;

    PriceLevel* level  =  hot_[ best_idx_ ];
    assert( level != nullptr );

    return level;
}

const PriceLevel*  HotPriceLevel::best_level() const noexcept {

    if ( best_idx_ == INVALID_ ) return nullptr;

    const PriceLevel* level  =  hot_[ best_idx_ ];
    assert( level != nullptr );

    return level;
}


PriceLevel*  HotPriceLevel::find ( Price price ) noexcept {

	if ( !contains( price ) ) return nullptr;

	PriceLevel* level  =  hot_[ to_idx( price ) ];

	if ( level == nullptr ) return nullptr;
	if ( level->price != price ) return nullptr;

	return level;
}


const PriceLevel*  HotPriceLevel::find ( Price price ) const noexcept {

	if ( !contains( price ) ) return nullptr;

	const PriceLevel* level  =  hot_[ to_idx( price ) ];

	if ( level == nullptr ) return nullptr;
	if ( level->price != price ) return nullptr;

	return level;
}


void  HotPriceLevel::promote ( Price price, PriceLevel* level ) noexcept {

	assert( level != nullptr );
	assert( contains( price ) );
	assert( level->price == price );

	const std::size_t idx  =  to_idx( price );
	PriceLevel* current  =  hot_[ idx ];

	// cant overwrite a live level
	assert( current == nullptr || current->price == price );

	hot_[ idx ]  =  level;

	update_best_after_add( price );
}


void  HotPriceLevel::demote ( Price price ) noexcept {

	if ( !contains( price ) ) return;

	const std::size_t idx  =  to_idx( price );

	PriceLevel* level  =  hot_[ idx ];

	if ( level == nullptr ) return;

	assert( level->price == price );

	hot_[ idx ]  =  nullptr;

	update_best_after_remove( price );
}


// window sliding ops
void  HotPriceLevel::center_window ( Price price ) noexcept {

	const Price half  =  static_cast<Price>( WINDOW_SIZE_ >> 1 );

	const Price new_low  =  price - half;
	const Price new_high =  new_low + static_cast<Price>( WINDOW_SIZE_ - 1 );

	assert( new_low  >= base_price_ );
	assert( new_high <  base_price_ + static_cast<Price>( NUM_LEVELS_ ) );

	window_low_  =  new_low;
	window_high_ =  new_high;
}


bool  HotPriceLevel::advance_window_up () noexcept {

	const Price new_high  =  window_high_ + static_cast<Price>( SLIDE_ );

	if ( !contains( new_high ) ) return false;

	window_low_  +=  static_cast<Price>( SLIDE_ );
	window_high_  =  new_high;

	return true;
}


bool  HotPriceLevel::advance_window_down () noexcept {

	const Price new_low  =  window_low_ - static_cast<Price>( SLIDE_ );

	if ( !contains( new_low ) ) return false;

	window_low_    =  new_low;
	window_high_  -=  static_cast<Price>( SLIDE_ );

	return true;
}


bool  HotPriceLevel::in_window ( Price p ) const noexcept {

	return p  >=  window_low_
	    && p  <=  window_high_;	
}
