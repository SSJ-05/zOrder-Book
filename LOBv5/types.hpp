// data types for LOB files// 04.07.26// ZeroK

#pragma once

#include <cstdint>

using Price     =  std::int64_t;
using Qty       =  std::uint32_t;
using OrderID   =  std::uint64_t;


// convert int to double helper func
constexpr double to_price (Price p) noexcept {

    return static_cast<double>( p ) / 100.00;
}
