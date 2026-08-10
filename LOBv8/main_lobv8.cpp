// main src file// 07.07.26// ZeroK

/* workflow
 * order generator -> order_pool -> arena -> 
 *      matching engine -> orderbook -> ringpriceladder -> trade
 */

#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "order_generator.hpp"
#include "matching_engine.hpp"

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include <immintrin.h>


constexpr std::size_t  NUM_TRADES  { 1 << 5 };



int main () {

    std::printf ("\n\n=== Session Open ===\n\n");

    std::printf ("size of order: %zu\n", sizeof(Order));  // debugging only


    OrderGenerator gen;
    MatchingEngine engine;


    auto start = __rdtsc();

    // for (auto _ {NUM_TRADES}; _-- > 0;) {
    for (auto i {0uz}; i < NUM_TRADES; ++i) {

        auto params  =  gen.next( order );
        engine.submit_order( params.externa_id, 
			     params.price,
			     params.qty,
			     params.side );

    }

    auto end = __rdtsc();

    std::printf( "\nCycles per order : %zu\n", (end - start) / NUM_TRADES);
    std::printf( "Engine book size   : %zu\n", engine.book_size() );

    // engine.print_book();     // for debugging only
    // engine.print_stats();

    std::printf ("\n\n=== zOrder Book Closed ===\n");
    std::printf ("\n\n=== Session Closed ===\n\n");

    return EXIT_SUCCESS;
}
