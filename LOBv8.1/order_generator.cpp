// order generator src file// 07.07.26// ZeroK

#include "orderv2.hpp"
#include "order_generator.hpp"

#include <algorithm>    // for std::clamp


OrderParams  OrderGenerator::next () {
    
    mid_price_ += static_cast<Price>( (fast_rand() % 5) - 2 );

    // optional clamp for keeping prices centered
    mid_price_ = std::clamp (
            mid_price_, Price {9000}, Price {13995} );

    OrderParams params;
    params.external_id  =  ++id_;
    params.price        =  mid_price_;
    params.qty          =  static_cast<Qty>( (fast_rand() % 100) + 1 );
    params.side   	=  ( fast_rand() & 1 ) ? Side::Bid : Side::Ask;

    return params;
}

