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

#include <cassert>


class PriceLevelIndex {

private:

	HotPriceLevel    hot_;
	ColdPriceLevel   cold_;
	PriceLevelStore  store_;


public:

	PriceLevel*  find ( Price price ) noexcept {

		if ( PriceLevel* level = hot_.find( price ) )
			return level;
				
		return  cold_.find( price );
	}


	PriceLevel*  acquire ( Price price ) noexcept {

		// check if price exists
		if ( PriceLevel* level = find( price ) )
			return level;

		// acquire physical storage
		PriceLevel* level  =  store_.acquire();

		if ( !level ) return nullptr;

		// estb level
		level->price  =  price;

		// put price level in hot or cold strc
		// acc to hot window pos
		if ( hot_.in_window( price ) ) { 

			hot_.promote( price, level );
			return level;
		}

		// cold insert can fail, since cold is bounded
		if ( cold_.insert( price, level ) == nullptr ) {

			store_.release( level );
			return nullptr;
		}

		return level;
	}


	void  release ( Price price ) noexcept {

		// try hot first
		if ( PriceLevel* level = hot_.find( price ) ) {

			hot_.demote( price );
			store_.release( level );
			return;
		}

		// then try cold
		if ( PriceLevel* level = cold_.find( price ) ) {

			cold_.erase( price );
			store_.release( level );
			return;
		}
	}

	PriceLevel*  best_level () noexcept;
};


