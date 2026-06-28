// order generator src file// 27.06.26// ZeroK

#include "order_generator.hpp"

Order OrderGenerator::next() {
    
    Order order {};

    order.price = mid_price_ + price_dist_(rng_) * 0.01;
    order.id    = ++id_;

    order.qty   = qty_dist_(rng_);

    order.side  = side_dist_(rng_) ?
                  Side::Buy : Side::Sell;

    return order;
}

