// ring price ladder header file// 13.08.26// ZeroK

#pragma once

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"

#include <array>
#include <cstdint>



class RingPriceLadder {

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

    // debug counters
#ifndef NDEBUG
	std::size_t  window_hits_   {};
	std::size_t  window_misses_ {};
#endif

    // storage for price levels
    std::array<PriceLevel, NUM_LEVELS_> rpl_;


public:

    // ctor
    explicit RingPriceLadder (Price base, Side side) 
	    : 	base_price_ (base),
        	side_ (side) {}


    // ops
    void  add          ( Order* ) noexcept;
    void  remove       ( Order* ) noexcept;
    void  clear_level  ( Price )  noexcept;

    void  update_best_after_add    ( Price ) noexcept;
    void  update_best_after_remove ( Price ) noexcept;

    std::size_t  to_idx   ( Price ) const noexcept;
    bool         contains ( Price ) const noexcept;

    const PriceLevel&  at_level ( Price ) const noexcept;
          PriceLevel&  at_level ( Price )       noexcept;

    const PriceLevel*  best_level()     const noexcept;
    	  PriceLevel*  best_level()           noexcept;


    // iteration interface
 	template <typename Fn>
	void for_each_level ( Fn&& fn ) const noexcept {

		for (auto i {0uz}; i < NUM_LEVELS_; ++i) {
		
			const auto& level = rpl_[i];

			if ( level.orders.empty() ) continue;

			fn ( base_price_ 
				+ static_cast<Price>( i ), level );
		}
	}  

    // keep the existing 8192 size ladder
    // and maintain an active window of size 2048
    void  center_window ( Price ) noexcept;
    bool  advance_window_up () noexcept;
    bool  advance_window_down () noexcept;
    bool  in_window ( Price ) const noexcept;


    // debug info
#ifndef NDEBUG
    void  print_stats() const noexcept;
#endif

};

