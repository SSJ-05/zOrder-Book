// main src file// 27.06.26// ZeroK

#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include "order_generator.hpp"

#include <cstdlib>
#include <cstdio>


int main (int argc, char** argv) {

    std::printf ("\n\n=== zOrder Book Open===\n\n");

    MatchingEngine engine;
    OrderGenerator gen;

    // for (auto i {0u}; i < 1000; ++i) {
    for (auto i {0x3e8}; i-- > 0;) {
        
        engine.submit ( gen ( next() ) );
    }

    std::printf ("\n\n=== zOrder Book Closed ===\n\n");

    return EXIT_SUCCESS;
}
