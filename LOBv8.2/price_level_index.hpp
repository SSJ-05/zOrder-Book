// price level index // 23.08.26 // ZeroK
/*
Orderbook
   │
   ▼
PriceLevelIndex
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

#include "ring_price_ladder.hpp"
#include "cold_price_level.hpp"
#include "price_level_store.hpp"


class PriceLevelIndex {

private:

	RingPriceLadder  hot_;
	ColdPriceLevel   cold_;
	PriceLevelStore  store_;


public:

	PriceLevel*  find ( Price price ) noexcept;
	PriceLevel*  acquire ( Price price ) noexcept;
	void  release ( Price price ) noexcept;
	PriceLevel*  best_level () noexcept;
};


