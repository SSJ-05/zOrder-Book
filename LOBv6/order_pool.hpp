// order pool // 19.07.26 // ZeroK
/* 
OrderPool
├── acquire Order
├── release Order
├── free-list management
└── occupancy bookkeeping

arena.hpp
    ↑
owns raw memory

order_pool.hpp
    ↑
allocates Orders from Arena
maintains freelist

matching_engine
    ↑
requests Order
returns Order
*/


#pragma once

#include "arena.hpp"
#include "orderv2.hpp"


class OrderPool {

private:
    Arena& arena_;

public:
    explicit OrderPool( Arena& arena )
        : arena_( arena ) {}

    Order* acquire() {
        return arena_.create<Order>();
    }

    void release( Order* ) {
        
    }
};

