// ring price ladder src file// 08.07.26// ZeroK

#pragma once

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
        
        return static_cast<std::size_t>( p - base_price_ )
               & MASK_;
}


bool RingPriceLadder::contains (Price p) const noexcept {
    
        return p >= base_price_
            && p <  base_price_ + static_cast<Price>( NUM_LEVELS_ );
}


// getter funcs for rpl_[index]
PriceLevel&  RingPriceLadder::at_level (Price p) noexcept {

    if ( contains (p) )
        return rpl_[to_idx( p )];
}


// void  RingPriceLadder::clear_level (Price new_price) noexcept {
//
//         auto& level     =  level (new_price);
//         level.price     =  new_price;
//         level.total_qty =  0;
//
//         level.orders.clear();
// }

