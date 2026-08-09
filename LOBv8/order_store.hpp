// order_store data structure // 09.08.26 // ZeroK
/* Notes: store dense storage and translate internal ID -> Order&
 
Gateway
    │
    │ ExternalID → InternalID
    ▼
OrderStore
    │
    │ InternalID → Order
    ▼
OrderBook

** gateway owns external ID -> internal ID mapping (use flat_map.hpp here)
** orderstore owns dense orders
** orderbook refs order ptr/internal IDs to active orders (replace flat_map with order_store)
** this skips hash lookup in hot path
** fixed capacity
** recycle internal IDs
** this replaces both flatmap and orderpool

*/

#pragma once

#include "orderv2.hpp"
#include "types.hpp"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <cassert>
#include <limits>


namespace zerok {

class OrderStore {

private:
	static constexpr std::size_t CAPACITY_   { 1 << 20 };
	static constexpr std::size_t WORD_SHIFT_ { 6 };
	static constexpr std::size_t NUM_WORDS_  { CAPACITY_ >> WORD_SHIFT_ };
	static constexpr std::size_t INVALID_ID_ { std::numeric_limits<InteralID>::max() };


	std::vector<Order> orders_;

	std::array<std::uint64_t, NUM_WORDS_> bitmap_;
	// 1 - free, 0 - allocated

	char pad_0 [ 64 ];
	alignas(64)
	std::size_t active_count_  {};

	alignas(64)
	std::size_t hint_word_  {};

public:
	explicit OrderStore ()
		: orders_ ( CAPACITY_ ) 
	{
		bitmap_.fill( ~0ULL );		// init with free slots
	}


	Order& operator[] ( InternalID id ) noexcept {
		
		assert( id < orders_.size() );
		return orders_[ id ];
	}


	const Order& operator[] ( InternalID id ) const noexcept {
		
		assert( id < orders_.size() );
		return orders_[ id ];
	}


	// acquire order
	[[ nodiscard ]]
	InternalID  acquire () noexcept {

		for (auto i {NUM_WORDS_}; i-- > 0;) {
		
			std::size_t word_idx  =  (hint_word_ - i) & (NUM_WORDS_ - 1);
			std::uint64_t word    =  bitmap_[ word_idx ];

			if ( word == 0 ) continue;

			unsigned bit  =  __builtin_ctzll( word );
			bitmap_[ word_idx ]  &=  ~( 1ULL << bit );  
	    		assert( (bitmap_[ word_idx ] & ( 1ULL << bit )) == 0 );

			hint_word_  =  word_idx;

			const std::size_t slot  =  (word_idx << WORD_SHIFT_) | bit;
			++active_count_;

			return  slot;
		}
		assert( false && "OrderStore is full.\n" );
		return INVALID_ID_;
	}

	// release order
	void  release ( InternalID id ) noexcept {

		const std::size_t slot  =  static_cast<std::size_t>( id );
		assert( slot < CAPACITY_ );

		const std::size_t word  =  slot >> WORD_SHIFT_;
		const std::size_t bit   =  slot & (BITS_PER_WORD - 1);

		assert( (bitmap_[ word ] & ( 1ULL << bit )) == 0 );
		bitmap_[ word ]  |=  ( 1ULL << bit ); 

		--active_count_;
	}


	// return the number of active orders
	std::size_t size() const noexcept {
		
		return active_count_;
	}

	std::size_t capacity() const noexcept {
	
		return orders_.size();
	}

};


} // namespace	

