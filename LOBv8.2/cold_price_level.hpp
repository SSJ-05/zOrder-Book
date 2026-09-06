// cold price levels header // 15.08.26 // ZeroK
/*
                 Orderbook
                     │
          ┌──────────┴──────────┐
          │                     │
     RingPriceLadder        ColdPriceLevel
       HOT 8192              COLD 2048
          │                     │
          │                     │
       PriceLevel*           PriceLevel*
          │                     │
          └──────────┬──────────┘
                     │
                PriceLevel
                     │
              IntrusiveList
                     │
                   Order
                     │
                OrderStore

ownership model
PriceLevelStore
       │
       ├── PriceLevel A
       ├── PriceLevel B
       ├── PriceLevel C
       └── ...

ColdPriceLevel
       │
       ├── {  9975, → A }
       ├── { 10032, → B }
       └── { 10481, → C }
*/

#pragma once

#include "price_level.hpp"
#include "types.hpp"

#include <array>	
#include <cstdint>
#include <algorithm>
#include <cassert>



class ColdPriceLevel {

private:

	static constexpr std::size_t COLD_CAPACITY_  { 1 << 11 };


	struct ColdEntry {
	
		Price 	     price  {};
		PriceLevel*  level  {};
	};


	std::array<ColdEntry, COLD_CAPACITY_> entries_  {};

	std::size_t size_  {};

	Side  side_;


public:

	explicit ColdPriceLevel ( Side side ) noexcept :
		side_ ( side ) {}


	// O (log N)
	[[ nodiscard ]]
	PriceLevel*  find ( Price price ) noexcept {

		auto it = std::lower_bound ( entries_.begin(),
			  	   	     entries_.begin() + size_,
				   	     price,
				   	     [] ( const ColdEntry& e, Price p ) {
							return e.price < p;
				   	     } );

		if ( it == entries_.begin() + size_ || it->price != price )
			return nullptr;		// not found

		return it->level;
	}

	[[ nodiscard ]]
	const PriceLevel*  find ( Price price ) const noexcept {

		const auto it = std::lower_bound ( entries_.begin(),
			  	   	     entries_.begin() + size_,
				   	     price,
				   	     [] ( const ColdEntry& e, Price p ) {
							return e.price < p;
				   	     } );

		if ( it == entries_.begin() + size_ || it->price != price )
			return nullptr;		// not found

		return it->level;
	}


	// O (N)
	PriceLevel*  insert ( Price price, PriceLevel* level ) noexcept {

		assert( level != nullptr );
		assert( size_ <= COLD_CAPACITY_ );

		if ( size_ >= COLD_CAPACITY_ ) return nullptr;

		// find the slot to insert
		auto end = entries_.begin() + size_;
		auto it = std::lower_bound ( entries_.begin(),
			  	   	     end,
				   	     price,
				   	     [] ( const ColdEntry& e, Price p ) {
							return e.price < p;
				   	     } );

		// price already exists in cold
		if ( it != end && it->price == price ) return nullptr;

		// shift elements to right
		for ( auto pos {end}; pos-- > it; ) 
			*pos  =  *(pos -1); 

		// insert
		it->price  =  price;
		it->level  =  level;
		++size_;

		return level;
	}


	// O (N)
	void  erase ( Price price ) noexcept {

		auto end = entries_.begin() + size_;
		auto it = std::lower_bound ( entries_.begin(),
			  	   	     end,
				   	     price,
				   	     [] ( const ColdEntry& e, Price p ) {
							return e.price < p;
				   	     } );

		if ( it == end || it->price != price ) return;		// not found

		// shift left
		for ( auto pos {it}; pos +1 != end; ++pos )
			*pos  =  *(pos +1);

		// erase the element
		entries_[ size_ -1 ]  =  ColdEntry {};
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


