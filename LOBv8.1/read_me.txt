// Limit Order Book LOBv8// 09.08.26// ZeroK

Files:
order_book/
│
├── order.hpp
├── trade.hpp
├── types.hpp
│
├── intrusive_list.hpp
├── intrusive_list.cpp
|
│-- ring_price_ladder.hpp
│-- ring_price_ladder.cpp
|
|-- order_store.hpp
|-- flat_map.hpp
|
├── price_level.hpp
│
├── orderbook.hpp
├── orderbook.cpp
│
├── matching_engine.hpp
├── matching_engine.cpp
│
└── main.cpp




**current model uptil v7
external order ID
        ↓
FlatMap
        ↓
Order*
        ↓
OrderBook


**proposed model in v8
             Gateway
                │
       external order ID
                │
                ▼
        External → Internal
             mapping in submit_order()
                │
                ▼
       dense OrderStore.acquire()
                │
                ▼
          Internal ID/ Order& 
                │
                ▼
	  OrderBook

**internal ID maps to dense storage idx

