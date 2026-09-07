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

	[[ nodiscard ]]
	PriceLevel*  find ( Price price ) noexcept {

		if ( PriceLevel* level = hot_.find( price ) )
			return level;
				
		return  cold_.find( price );
	}


	[[ nodiscard ]]
	PriceLevel*  acquire ( Price price ) noexcept {

		// check if price exists
		if ( PriceLevel* level = find( price ) )
			return level;

		// acquire physical storage
		PriceLevel* level  =  store_.acquire();

		if ( !level ) return nullptr;

		// estb level
		level->price  	  =  price;
		level->total_qty  =  0;

		// put price level in hot or cold strc
		// acc to hot window pos
		if ( hot_.in_window( price ) ) { 

			hot_.promote( price, level );
			return level;
		}

		// cold insert can fail, since cold is bounded
		PriceLevel* inserted  =  cold_.insert( price, level );

		if ( inserted == nullptr ) {

			assert( level->orders.empty() );
			assert( level->total_qty == 0 );

			store_.release( level );
			return nullptr;
		}

		// if this price becomes global best
		// slide the hot window 

		return inserted;
	}


	[[ nodiscard ]]
	void  release ( Price price ) noexcept {

		// try hot first
		if ( PriceLevel* level = hot_.find( price ) ) {

			hot_.demote( price );

			assert( level->orders.empty() );
			assert( level->total_qty == 0 );

			store_.release( level );
			return;
		}

		// then try cold
		if ( PriceLevel* level = cold_.find( price ) ) {

			cold_.erase( price );

			assert( level->orders.empty() );
			assert( level->total_qty == 0 );

			store_.release( level );
			return;
		}
	}


	// invariant: global best_level will always be promoted to hot window
	// and hot window slides to accommodate best_level to maintain the invariant
	[[ nodiscard ]]
	PriceLevel*  best_level () noexcept { return  hot_.best_level(); }

	[[ nodiscard ]]
	const PriceLevel*  best_level () const noexcept { return  hot_.best_level(); }
};


