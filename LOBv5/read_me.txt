// Limit Order Book LOBv5// 08.07.26// ZeroK

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
├── price_level.hpp
│
├── orderbook.hpp
├── orderbook.cpp
│
├── matching_engine.hpp
├── matching_engine.cpp
│
└── main.cpp


OrderGenerator
      │
      ▼
Arena::create<Order>()
      │
      ▼
MatchingEngine::submit(Order*)
      │
      ▼
Orderbook::add_order(Order*)
      │
      ▼
IntrusiveList



v5 architecture:
MatchingEngine
        │
        ▼
Orderbook
        │
        ├───────────────┐
        ▼               ▼
 BidLadder         AskLadder
        │               │
        ├── add()       ├── add()
        ├── remove()    ├── remove()
        ├── best()      ├── best()
        ├── update()    ├── update()
        └── window()    └── window()



**architectural separation:

- Orderbook coordinates the engine.
- RingPriceLadder owns one side of the book.
- PriceLevel owns FIFO.
- IntrusiveList owns linking.
- Arena owns memory.
