// cold price levels header // 15.08.26 // ZeroK

#pragma once

#include "price_level.hpp"
#include "types.hpp"

#include <array>	
#include <cstdint>
#include <algorithm>


namespace zerok {

class ColdPriceLevel {

private:

	static constexpr std::size_t COLD_CAPACITY_  { 1 << 11 };


	struct ColdEntry {
	
		Price 	     price;
		PriceLevel*  level;
	};


	std::array<PriceLevel, COLD_CAPACITY_> entries_  {};

	std::size_t size_  {};


public:

	PriceLevel*  find ( Price price ) const noexcept {

		auto* val  =  
			std::lower_bound ( entries_.begin(),
				   	   entries_.begin() + size_,
				   	   price,
				   	   [] ( const ColdEntry& e, Price p ) {
						e.price < p;
				   	   } );

		return val;
	}

	PriceLevel*  insert ( Price p, PriceLevel* level ) noexcept;
	void  erase ( Price p ) noexcept;
	std::size_t  size () const noexcept { return size_; }

	std::size_t  capacity () const noexcept { return CAPACITY_; }

};

}	// namespace
