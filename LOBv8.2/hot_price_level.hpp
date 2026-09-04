// ring price ladder header file// 15.08.26// ZeroK
/*
 * ownership model
HotPriceLevel
    └── hot price index
         └── PriceLevel*

ColdPriceLevel
    └── cold price index
         └── PriceLevel*

PriceLevelStore
    └── owns actual PriceLevel objects

OrderStore
    └── owns actual Order objects─
*/

#pragma once

#include "types.hpp"
#include "orderv2.hpp"
#include "price_level.hpp"

#include <array>
#include <cstdint>


class HotPriceLevel {

private:

    // constants
    static constexpr std::size_t  NUM_LEVELS_  { 1 << 13 };
    static constexpr std::size_t  MASK_        { NUM_LEVELS_ - 1 };
    static constexpr std::size_t  INVALID_     { NUM_LEVELS_ };


    // sliding window invariants
    static constexpr std::size_t  WINDOW_SIZE_  { 1 << 11 };
    static constexpr std::size_t  SLIDE_	{ 1 << 9 };	

    Price 	window_low_   {};
    Price 	window_high_  {};


    // ring invariants
    Price        base_price_  {};
    Side         side_        { Side::Bid };
    std::size_t  best_idx_    { INVALID_ };

    // hot idx only - dosnt own PriceLevel	
    std::array<PriceLevel*, NUM_LEVELS_>  hot_  {};


public:
    
    // debug counters
#ifndef NDEBUG
	std::size_t  window_hits_   {};
	std::size_t  window_misses_ {};
#endif


    // ctor
    explicit HotPriceLevel (Price base, Side side) 
	    : 	base_price_ (base),
        	side_ (side),
		window_low_ (base),
		window_high_ (base + static_cast<Price>( WINDOW_SIZE_ - 1 )) 
    {}


    // level add/remove from hot_
    void  promote   ( Price, PriceLevel* ) noexcept;
    void  demote    ( Price ) 		   noexcept;

    // matching ops
    void  update_best_after_add    ( Price ) noexcept;
    void  update_best_after_remove ( Price ) noexcept;

    std::size_t  to_idx   ( Price ) const noexcept;
    bool         contains ( Price ) const noexcept;	// ring range

    // const PriceLevel&  at_level ( Price ) const noexcept;
    //       PriceLevel&  at_level ( Price )       noexcept;

    const PriceLevel*  best_level()     const noexcept;
    	  PriceLevel*  best_level()           noexcept;

    const PriceLevel*  find ( Price ) 	const noexcept;
    	  PriceLevel*  find ( Price ) 	      noexcept;



    	// iteration interface
 	template <typename Fn>
	void for_each_level ( Fn&& fn ) const noexcept {

		for (auto i {0uz}; i < NUM_LEVELS_; ++i) {
		
			const PriceLevel* level = hot_[i];

			if ( level == nullptr ) continue;

			fn ( level->price, *level );
		}
	}  

    // keep the existing 8192 size ladder
    // and maintain an active window of size 2048
    void  center_window ( Price ) noexcept;
    bool  advance_window_up () noexcept;
    bool  advance_window_down () noexcept;
    bool  in_window ( Price ) const noexcept;	// price in active window
						// diff from contains()


    // debug info
#ifndef NDEBUG
    void  print_stats() const noexcept;
#endif

};

