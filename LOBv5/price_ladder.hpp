// price ladder header file// 08.07.26// ZeroK

#pragma once

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"

#include <array>
#include <cstdint>



struct PriceLadder {

    Price base_price_ {};
    static constexpr std::size_t NUM_LEVELS  { 1 << 12 };

    std::array<PriceLevel, NUM_LEVELS> ladder_ {};


    // // ctor
    // explicit PriceLadder (Price min, Price max) :
    //     MIN_PRICE (min), MAX_PRICE (max) {}


    // helper func to calculate index
    std::size_t to_idx (Price p) const noexcept {
        
        return static_cast<std::size_t>( p - base_price_ );
    }


    bool contains (Price p) const noexcept {
    
        return p >= base_price_ 
            && p <  base_price_ 
                    + static_cast<std::size_t>( NUM_LEVELS );
    }
};

