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
#include "arena.hpp"
#include "order_pool.hpp"

#include <cstdlib>
#include <cstdio>
#include <cassert>


constexpr std::size_t  ARENA_CAPACITY  { 1 << 15 };
constexpr std::size_t  MAX_ORDERS      { 2 * ARENA_CAPACITY };
constexpr std::size_t  NUM_TRADES      { 1 << 10 };



int main () {

    std::printf ("\n\n=== Session Open ===\n\n");

    std::printf ("size of order: %zu\n", sizeof(Order));  // debugging only

    Arena arena (MAX_ORDERS * sizeof( Order ));
    OrderPool pool (arena);

    MatchingEngine engine (pool);
    OrderGenerator gen;


    for (auto _ {NUM_TRADES}; _-- > 0;) {
        
        Order* order = pool.acquire();

        gen.next( order );
        engine.submit_order( order );
    }


    engine.print_book();     // for debugging only

    // arena.reset();   // for multiple sessions only

    std::printf ("Arena Stats:\n"
		    "\tused     : %zu\n"
		    "\tcapacity : %zu\n",
                arena.used() >> 10, arena.capacity() >> 10);

    std::printf ("\n\n=== zOrder Book Closed ===\n");
    std::printf ("\n\n=== Session Closed ===\n\n");

    return EXIT_SUCCESS;
}
