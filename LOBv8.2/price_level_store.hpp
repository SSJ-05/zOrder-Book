// price level store header file // 16.08.26 // ZeroK
// stores PriceLevels, which will be indexed by either hot or cold strc


#pragma once

#include "price_level.hpp"
#include "types.hpp"

#include <array>
#include <cstdint>
#include <cassert>


namespace zerok {

class PriceLevelStore {

private:
	
	static constexpr std::size_t CAPACITY_    	{ 1 << 12 };
	static constexpr std::size_t WORD_SHIFT_  	{ 6 };
	static constexpr std::size_t BITS_PER_WORD_   	{ 1 << WORD_SHIFT_ };
	static constexpr std::size_t NUM_WORDS_   	{ CAPACITY_ >> WORD_SHIFT_ };

	static_assert( (CAPACITY_ & (CAPACITY_ -1)) == 0,
			"CAPACITY_ must be a multiple of 64" );
	static_assert( (NUM_WORDS_ & (NUM_WORDS_ -1)) == 0,
			"NUM_WORDS_ must be power of 2" );


	// storage for price levels
	std::array<PriceLevel, CAPACITY_> levels_;

	// free - 1, occupied - 0
	std::array<std::uint64_t, NUM_WORDS_> bitmap_;


	// stats counter
	std::size_t  active_count_ {};		// count no. of active levels in data strc

	std::size_t  hint_word_ {};


public:

	PriceLevelStore() 
	{
		bitmap_.fill( ~0ULL );
	}


	[[ nodiscard ]]
	inline
	PriceLevel*  acquire() noexcept {

		for ( auto i {0uz}; i < NUM_WORDS_; ++i ) {

			std::size_t  word_idx  =  (hint_word_ + i) & (NUM_WORDS_ - 1);
			std::uint64_t word     =  bitmap_[ word_idx ];

			if ( word == 0 ) continue;

			const unsigned bit  =  __builtin_ctzll( word );
			bitmap_[ word_idx ] &= ~( 1ULL << bit );

			const std::size_t index  =  (word_idx << WORD_SHIFT_) | bit;
			
			++active_count_;
			hint_word_  =  word_idx;

			PriceLevel* level  =  &levels_[ index ];

			*level  =  PriceLevel {};	// reset the state

			return level;
		} 
		return nullptr;
	}


	inline
	void  release ( PriceLevel* level ) noexcept {

		assert( level != nullptr );

		assert( level >= levels_.data() );
		assert( level < levels_.data() + CAPACITY_ );

		// core invariants
		assert( level->orders.empty() );
		assert( level->total_qty == 0 );

		const std::size_t index  =  
			static_cast<std::size_t>( level - levels_.data() );

		const std::size_t word  =  index >> WORD_SHIFT_;

		const unsigned bit  =  index & (BITS_PER_WORD_ - 1);

		// bit must be set to 0 before it is set to 1
		assert( (bitmap_[ word ] & (1ULL << bit)) == 0 );

		*level  =  PriceLevel {};

		bitmap_[ word ]  |=  ( 1ULL << bit );	// set bit to 1

		--active_count_;

		if ( word < hint_word_ ) 
			hint_word_  =  word;
	}



	[[ nodiscard ]]
	inline
	std::size_t  active_count () const noexcept {
	
		return  active_count_;
	}


	[[ nodiscard ]]
	inline
	std::size_t  capacity () const noexcept {
	
		return  CAPACITY_;
	}

};

} // namespace

