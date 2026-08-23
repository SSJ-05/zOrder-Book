// price level index // 23.08.26 // ZeroK
/*
Orderbook
   │
   ▼
PriceLevelIndex	(routing layer)
   │
   ├── RingPriceLadder      ← hot price discovery
   │
   └── ColdPriceLevel       ← sparse extremes
           │
           ▼
     PriceLevelStore        ← owns PriceLevels
           │
           ▼
      PriceLevel
           │
           ▼
      IntrusiveList
           │
           ▼
         Order
           │
           ▼
       OrderStore
*/

#pragma once

#include "hot_price_level.hpp"
#include "cold_price_level.hpp"
#include "price_level_store.hpp"

#include <array>	
#include <cstdint>
#include <algorithm>
#include <cassert>


class PriceLevelIndex {

private:

	HotPriceLevel    hot_;
	ColdPriceLevel   cold_;
	PriceLevelStore  store_;


public:

	PriceLevel*  find ( Price price ) noexcept {

		if ( hot_.in_window( price ) )
			return  hot_.find( price );
		
		return  cold_.find( price );
	}


	PriceLevel*  acquire ( Price price ) noexcept {

		// check if price exists
		if ( PriceLevel* level = find( price ) )
			return level;

		PriceLevel* level  =  store_.acquire();

		if ( !level ) return nullptr;

		// put price level in hot or cold strc
		if ( hot_.in_window( price ) ) 
			hot_.insert( price, level );
		else
			cold_.insert( price, level );


		return level;
	}


	void  release ( Price price ) noexcept {

		PriceLevel* level  =  find( price );

		if ( !level ) return;

		// remove price from whichever level hot or cold
		if ( hot_.in_window( price ) )
			hot_.erase( price );
		else 
			cold_.erase( price );

		// return actual price level
		store_.release( level );
	}

	PriceLevel*  best_level () noexcept;
};


