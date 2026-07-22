// order pool // 19.07.26 // ZeroK
/* 
OrderPool
├── acquire Order
├── release Order
├── free-list management
└── occupancy bookkeeping

arena.hpp
    ↑
owns raw memory

order_pool.hpp
    ↑
allocates Orders from Arena
maintains freelist

matching_engine
    ↑
requests Order
returns Order
*/


#pragma once

#include "arena.hpp"
#include "orderv2.hpp"

#include <cstdint>
#include <array>
#include <cassert>


class OrderPool {

private:
    static constexpr std::size_t  CAPACITY_      { 1 << 15 };
    static constexpr std::size_t  BITS_PER_WORD_ { 64 };

    // divide by 64 (right shift by 6) 
    static constexpr std::size_t  WORD_SHIFT_    { 6 };
    static constexpr std::size_t  NUM_WORDS_     { CAPACITY_ >> WORD_SHIFT_ };    
    // uint64_t is 64 bit, capacity divided by 64 gives us x slots
    // slot 1: bitmap_[0] - bits 0-63   - orders 0-63
    // slot 2: bitmap_[1] - bits 64-127 - order 64-127....
    // slot x: bitmap_[x] - bits 128-(x-1) - orders 128-(x-1)


    Arena&  arena_;
    Order*  base_;

    static_assert( (CAPACITY_ & (BITS_PER_WORD_ - 1)) == 0, 
		    "CAPACITY must be multiple of 64\n" );

    std::array<std::uint64_t, NUM_WORDS_> bitmap_;
    // 1: free, 0: allocated

    std::size_t hint_word_ {};  // start iteration from last known free word
                                // avoids starting iteration from 0 again
                                // use in acquire()


public:
    explicit OrderPool( Arena& arena )
        : arena_( arena ),
          base_( arena_.allocate_array<Order>( CAPACITY_ ) ) 
    {
          bitmap_.fill( ~0ULL );        // every slot is free
          assert( base_ != nullptr );
    }


    [[ nodiscard ]]
    Order* acquire() noexcept {

        // for (std::size_t i{}; i < NUM_WORDS_; ++i) {
        for (std::size_t i {NUM_WORDS_}; i-- > 0;) {

            std::size_t   word_idx = (hint_word_ + i) & (NUM_WORDS_ - 1);  // wrap around logic
            std::uint64_t word     = bitmap_[ word_idx ];   // word: 64-bit bitmap value

            if (word == 0) continue;

            unsigned bit  =  __builtin_ctzll( word );
            bitmap_[ word_idx ]  &=  ~( 1ULL << bit );

            hint_word_  =  word_idx;    // update hint for next iteration

            const std::size_t slot = word_idx * BITS_PER_WORD_ + bit;
            // const std::size_t slot = (word_idx << BITS_PER_WORD_) | bit;
            
            return new ( &base_[ slot ] ) Order{};
            // placement new, bcoz allocate_array returns raw memory
            // construct order before returning
        }

        assert( false && "OrderPool exhausted\n" );
        return nullptr;     // out of memory
    }


    void release( Order* order ) noexcept {

        assert( order != nullptr );
        assert( order >= base_ );
        assert( order < base_ + CAPACITY_ );

        // 1. destroy the order (placement new in line 93)
        order-> ~Order();

        // 2. compute slot idx
        const std::size_t slot = static_cast<std::size_t>( order - base_ );
        assert( slot < CAPACITY_ );

        // 3. locate bitmap word
        const std::size_t word  =  slot >> WORD_SHIFT_;     // divide by 64
        const std::size_t bit   =  slot & (BITS_PER_WORD_ - 1);

        // 4. mark the bit free
        bitmap_[ word ]  |=  ( 1ULL << bit );
    }

};

