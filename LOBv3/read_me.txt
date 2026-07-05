// Limit Order Book LOB// 27.06.26// ZeroK

Files:
order_book/
├── order.hpp
├── trade.hpp

|--- types.hpp
|--- price_level.hpp

├── order_book.hpp
├── order_book.cpp

├── matching_engine.hpp
├── matching_engine.cpp

├── order_generator.hpp
├── order_generator.cpp

└── main.cpp


workflow of v3:
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
