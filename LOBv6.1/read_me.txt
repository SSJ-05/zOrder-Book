// Limit Order Book LOBv6// 19.07.26// ZeroK

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



**Architectural separation:
- Orderbook coordinates the engine.
- RingPriceLadder owns one side of the book.
- PriceLevel owns FIFO.
- IntrusiveList owns linking.
- Arena owns memory.



***New features
1. free list bitmask
2. order recycling
    currently : after matching, memory goes to waste
    V6 : after matching, dtor called, memory block returns to free list
3. occupancy bitmap in ring
    currently : linear scan for best level after removal
    V6 : bitmap -> ctz, clz bit hacking -> best level in O(1)
4. split market data from publishing
5. benchmark against V5

