// order generator src file// 27.06.26// ZeroK

#include "order_generator.hpp"

// #include <algorithm>    // for std::clamp

Order OrderGenerator::next() {
    
    Order order {};

    // Price spread_ { 2 };

    mid_price_ += price_dist_(rng_);

    // // optional clamp for keeping prices centered
    // mid_price_ = std::clamp (
    //         mid_price_, Price {9900}, Price {10100} );

    order.price = mid_price_;
    
    order.id    = ++id_;

    order.qty   = qty_dist_(rng_);

    order.side  = side_dist_(rng_) ?
                  Side::Bid : Side::Ask;

    return order;
}

