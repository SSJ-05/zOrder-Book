// matching engine header file// 27.06.26// ZeroK

#pragma once

#include "orderbook.hpp"

class MatchingEngine {

private:
    Orderbook book_;

public:
    void submit_order (const Order& order);
};
