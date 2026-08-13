// order generator header file// 27.06.26// ZeroK

#pragma once

#include "types.hpp"
#include "orderv2.hpp"


struct OrderParams {
	
	OrderID external_id;
	Price   price;
	Qty	qty;
	Side	side;
};


class OrderGenerator {

private:

    OrderID       id_;
    Price     	  mid_price_;
    std::uint64_t rng_state_;


    std::uint64_t fast_rand () {
	
	    rng_state_ ^=  rng_state_ << 13;
    	    rng_state_ ^=  rng_state_ >> 7;
            rng_state_ ^=  rng_state_ << 17;
    	    
	    return rng_state_;
    }


public:
    OrderGenerator() :
        id_ (1),		// start generation with order id 01
        mid_price_ (10000),
        rng_state_ ( 42 ) {}


    OrderParams next ();
};

