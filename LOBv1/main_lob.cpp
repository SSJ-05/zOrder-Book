// main src file// 27.06.26// ZeroK

/* workflow
 * order generator -> ring 
 *      -> matching engine -> trade ring + book updates -> publisher
 * */

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "order_generator.hpp"
#include "matching_engine.hpp"

#include <cstdlib>
#include <cstdio>


constexpr int NUM_TRADES { 0x10 };


int main () {

    std::printf ("\n\n=== zOrder Book Open===\n\n");

    std::printf ("size of order: %zu\n", sizeof(Order));
    MatchingEngine engine;
    OrderGenerator gen;


    for (auto i {NUM_TRADES}; i-- > 0;) {
        
        engine.submit_order ( gen.next() );
    }

    engine.print_book();

    std::printf ("\n\n=== zOrder Book Closed ===\n\n");

    return EXIT_SUCCESS;
}
