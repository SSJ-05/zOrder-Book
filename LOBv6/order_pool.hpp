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
    static constexpr std::size_t  CAPACITY      { 1 << 10 };
    static constexpr std::size_t  NUM_WORDS     { CAPACITY >> 6 };    // right shift by 6 (divide by 64)
    static constexpr std::size_t  BITS_PER_WORD { 64 };
    // uint64_t is 64 bit, capacity divided by 64 gives us x slots
    // slot 1: bitmap_[0] - bits 0-63   - orders 0-63
    // slot 2: bitmap_[1] - bits 64-127 - order 64-127....
    // slot x: bitmap_[x] - bits 128-(x-1) - orders 128-(x-1)


    Arena& arena_;
    Order* base_;

    static_assert( (CAPACITY & (BITS_PER_WORD - 1)) == 0, "CAPACITY must be multiple of 64\n" );
    std::array<std::uint64_t, NUM_WORDS> bitmap_;
    // 1: free, 0: allocated

public:
    explicit OrderPool( Arena& arena)
        : arena_( arena ),
          base_( arena_.allocate_array<Order>( CAPACITY ) ) 
    {
          bitmap_.fill( ~0ULL );        // every slot is free
          assert( base_ != nullptr );
    }


    Order* acquire() {

        for (std::size_t word_idx {}; word_idx < NUM_WORDS; ++word_idx) {

            std::uint64_t word = bitmap_[ word_idx ];   // word: 64-bit bitmap value

            if (word == 0) continue;

            unsigned bit  =  __builtin_ctzll( word );
            bitmap_[ word_idx ]  &=  ~( 1ULL << bit );

            const std::size_t slot = word_idx * BITS_PER_WORD + bit;
            Order* order = &base_[ slot ];
            new (order) Order{};    // placement new, bcoz allocate_array returns raw memory
                                    // construct order before returning
            return order;
        }

        assert( false && "OrderPool exhausted\n" );
        return nullptr;
    }


    void release( Order* ) {
        // To do : bitmap freelist
    }
};

