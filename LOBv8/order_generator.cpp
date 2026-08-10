// order generator src file// 07.07.26// ZeroK

#include "orderv2.hpp"
#include "order_generator.hpp"

#include <algorithm>    // for std::clamp


OrderParams  OrderGenerator::next () {
    
    mid_price_ += price_dist_(rng_);

    // optional clamp for keeping prices centered
    mid_price_ = std::clamp (
            mid_price_, Price {9000}, Price {13995} );

    OrderParams params;
    params.external_id  =  ++id_;
    params.price        =  mid_price_;
    params.qty          =  qty_dist_(rng_);
    params.side   	=  side_dist_(rng_) ?
                     	   Side::Bid : Side::Ask;


    return params;
}

