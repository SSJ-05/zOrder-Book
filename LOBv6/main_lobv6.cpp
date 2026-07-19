// main src file// 07.07.26// ZeroK

/* workflow
 * order generator -> arena -> matching engine ->
 *          orderbook -> ringpriceladder -> trade
 */

#include "orderv2.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "order_generator.hpp"
#include "matching_engine.hpp"
#include "arena.hpp"

#include <cstdlib>
#include <cstdio>
#include <cassert>


constexpr std::size_t ARENA_CAPACITY  { 1 << 15 };
constexpr std::size_t MAX_ORDERS      { 2 * ARENA_CAPACITY };
constexpr std::size_t NUM_TRADES      { 1 << 9 };



int main () {

    std::printf ("\n\n=== Session Open ===\n\n");

    std::printf ("size of order: %zu\n", sizeof(Order));
    MatchingEngine engine;
    OrderGenerator gen;

    Arena arena (MAX_ORDERS * sizeof(Order));


    for (auto _ {NUM_TRADES}; _-- > 0;) {
        
        Order* order = arena.create<Order>();
        gen.next( order );
        engine.submit_order( order );
    }

    // engine.print_book();     // for debugging only

    // arena.reset();   // for multiple sessions only

    std::printf ("Arena: %zu / %zu\n", 
                arena.used(), arena.capacity());

    std::printf ("\n\n=== zOrder Book Closed ===\n");
    std::printf ("\n\n=== Session Closed ===\n\n");

    return EXIT_SUCCESS;
}
