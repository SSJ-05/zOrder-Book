// cold price levels header v2 // 06.09.26 // ZeroK
// uses branchless lower bound BLB (invariant : range shud be power of 2
// COLD_CAPACITY_ is power of 2, but size_ is not
// therefore, for BLB invariant to work, we fill array with dummy ColdEntry's


#pragma once

#include "price_level.hpp"
#include "types.hpp"

#include <array>	
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <limits>



class ColdPriceLevel {

private:

	static constexpr std::size_t COLD_CAPACITY_  { 1 << 11 };
	static constexpr Price       DUMMY_PRICE_    { std::numeric_limits<Price>::max() };


	struct ColdEntry {
	
		Price 	     price  {};
		PriceLevel*  level  {};
	};


	std::array<ColdEntry, COLD_CAPACITY_> entries_  {};

	std::size_t size_  {};

	Side  side_  { Side::Bid };


	// custom branchless lower bound for LOB
	[[ nodiscard ]]
	__attribute__(( hot ))
	static
	std::size_t  BLB ( const ColdEntry* entries, Price price ) noexcept {

		auto pos { 0uz };

		for ( auto step {COLD_CAPACITY_}; (step >>= 1) > 0; ) {

			const auto val  =  pos | step;

			pos  =  ( entries[ val -1 ].price < price )
				? val 
				: pos;
		}

		return pos;
	}


public:

	explicit ColdPriceLevel ( Side side ) noexcept 
		: side_ ( side ) 
	{
		for ( auto& entry : entries_ ) {
			entry.price  =  DUMMY_PRICE_;
			entry.level  =  nullptr;
		}
	}


	// O (log N)
	[[ nodiscard ]]
	PriceLevel*  find ( Price price ) noexcept {

		const std::size_t idx  =  BLB( entries_.data(), price );

		if ( idx >= size_ ) return nullptr;
		if ( entries_[ idx ].price != price ) return nullptr;

		return  entries_[ idx ].level;
	}

	[[ nodiscard ]]
	const PriceLevel*  find ( Price price ) const noexcept {

		const std::size_t idx  =  BLB( entries_.data(), price );

		if ( idx >= size_ ) return nullptr;
		if ( entries_[ idx ].price != price ) return nullptr;

		return  entries_[ idx ].level;
	}


	// O (N)
	PriceLevel*  insert ( Price price, PriceLevel* level ) noexcept {

		assert( level != nullptr );
		assert( size_ <= COLD_CAPACITY_ );

		if ( size_ >= COLD_CAPACITY_ ) return nullptr;

		// find the slot to insert
		const std::size_t idx  =  BLB( entries_.data(), price );

		// price already exists in cold
		if ( idx < size_ 
		     && entries_[ idx ].price == price ) return nullptr;

		// shift elements to right
		for ( auto pos {size_}; pos-- > idx; ) {
			entries_[ pos ]  =  entries_[ pos -1 ];
		}

		// insert
		entries_[ idx ].price  =  price;
		entries_[ idx ].level  =  level;
		++size_;

		return level;
	}


	// O (N)
	void  erase ( Price price ) noexcept {

		const std::size_t idx  =  BLB( entries_.data(), price );

		if ( idx >= size_ 
		     || entries_[ idx ].price != price ) return;
		
		// shift left
		for ( auto pos {idx}; pos +1 < size_; ++pos ) {

			entries_[ pos ]  =  entries_[ pos +1 ];
		}

		// erase the element
		entries_[ size_ -1 ]  =  ColdEntry { 
						.price  =  DUMMY_PRICE_,
						.level  =  nullptr 
					 };
		--size_;
	}


	// O(1)
	[[ nodiscard ]]
	PriceLevel*  best_level () noexcept {

		if ( size_ == 0 ) return nullptr;

		if ( side_ == Side::Bid ) 
			return  entries_[ size_ -1 ].level;

		return  entries_[ 0 ].level;
	}
	
	[[ nodiscard ]]
	const PriceLevel*  best_level () const noexcept {

		if ( size_ == 0 ) return nullptr;

		if ( side_ == Side::Bid ) 
			return  entries_[ size_ -1 ].level;

		return  entries_[ 0 ].level;
	}


	[[ nodiscard ]]
	std::size_t  size () const noexcept { return size_; }

	[[ nodiscard ]]
	std::size_t  capacity () const noexcept { return COLD_CAPACITY_; }

};


