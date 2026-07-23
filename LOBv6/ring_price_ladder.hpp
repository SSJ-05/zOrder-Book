// ring price ladder header file// 08.07.26// ZeroK

#pragma once

#include "orderv2.hpp"
#include "price_level.hpp"
#include "types.hpp"

#include <array>
#include <cstdint>



class RingPriceLadder {

private:

    static constexpr std::size_t NUM_LEVELS_  { 1 << 12 };
    static constexpr std::size_t MASK_        { NUM_LEVELS_ - 1 };
    static constexpr std::size_t INVALID_     { NUM_LEVELS_ };

    Price        base_price_  {};
    Side         side_        { Side::Bid };
    std::size_t  best_idx_    { INVALID_ };

    std::array<PriceLevel, NUM_LEVELS_> rpl_;


public:

    // ctor
    explicit RingPriceLadder (Price base, Side side) 
	    : 	base_price_ (base),
        	side_ (side) {}


    // ops
    void  add          (Order*) noexcept;
    void  remove       (Order*) noexcept;
    void  clear_level  (Price)  noexcept;

    void  update_best_after_add    (Price) noexcept;
    void  update_best_after_remove (Price) noexcept;

    std::size_t  to_idx   (Price) const noexcept;
    bool         contains (Price) const noexcept;

    const PriceLevel&  at_level (Price) const noexcept;
          PriceLevel&  at_level (Price)       noexcept;

    const PriceLevel*  best_level()     const noexcept;
    	  PriceLevel*  best_level()           noexcept;


    // iteration interface
 	template <typename Fn>
	void for_each_level (Fn&& fn) const noexcept {

		for (auto i {0uz}; i < NUM_LEVELS_; ++i) {
		
			const auto& level = rpl_[i];

			if ( level.orders.empty() ) continue;

			fn ( base_price_ 
				+ static_cast<Price>( i ), level );
		}
	}  

    // reserved for future versions
    // void advance_window (Price new_base);
};

