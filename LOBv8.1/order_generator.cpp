// order generator src file// 07.07.26// ZeroK

#include "orderv2.hpp"
#include "order_generator.hpp"

#include <algorithm>    // for std::clamp


OrderParams  OrderGenerator::next () {
    
	// const auto r = fast_rand();
	//
	// if ( (r % 100) == 0 ) {
	//
	// 	const Price jump = 
	// 		static_cast<Price>( fast_rand() % 2049 ) - 1024;
	// 	mid_price_ += jump;
	// }
	// else {
	//
	// 	mid_price_ += 
	// 		static_cast<Price>( fast_rand() % 5 ) - 2;
	// }

	mid_price_ += static_cast<Price>( fast_rand() % 5 ) - 2;

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

