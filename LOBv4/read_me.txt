// Limit Order Book LOBv4// 05.07.26// ZeroK

Files:
order_book/
│
├── order.hpp
├── trade.hpp
├── types.hpp
│
├── intrusive_list.hpp
├── intrusive_list.cpp
│
├── price_level.hpp
│
├── orderbook.hpp
├── orderbook.cpp
│
├── matching_engine.hpp
├── matching_engine.cpp
│
└── main.cpp


workflow of v4:
              OrderID
                 │
                 ▼
         unordered_map
                 │
                 ▼
       OrderLocation
        ┌────┴────┐
        │         │
     Side      Price
        │         │
        └────┬────┘
             ▼
      map<Price,Level>
             │
             ▼
      list/deque<Order>
