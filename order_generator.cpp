// order generator src file// 27.06.26// ZeroK

#include "order_generator.hpp"

Order OrderGenerator::next() {
    
    Order order {};

    mid_price += ud_(rng_) * 0.01;
    order.id = ++id_;

    order.price = mid_price_ + 0.01;
    order.qty   = mid_qty + 5;

    return order;
}
