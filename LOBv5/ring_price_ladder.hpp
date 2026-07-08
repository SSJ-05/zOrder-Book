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

    Price  best_idx_    {};
    Price  base_price_  {};

    std::array<PriceLevel, NUM_LEVELS_> rpl_ {};


public:
    // ctor
    explicit RingPriceLadder (Price base) :
        base_price_ (base) {}

    // ops
    void add (Order*) noexcept;

    void remove (Order*) noexcept;

    void clear_level (Price new_price) noexcept;

    void update_best() noexcept;

    std::size_t to_idx (Price p) const noexcept;

    bool contains (Price p) const noexcept;

    PriceLevel& at_level (Price p) noexcept;

    PriceLevel* best_level ();

   
    // reserved for future versions
    void advance_window (Price new_base);
};

