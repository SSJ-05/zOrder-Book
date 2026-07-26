// flat map // 26.07.26 // ZeroK
// will replace std::unordered_map in orderbook for order_map_


#pragma once

namespace zerok {

template <
	typename Key,
	typename Value,
	std::size_t Capacity
>
class flat_map {

	constexpr std::size_t  MASK  { Capacity - 1 };

	static_assert( Capcity & MASK == 0 );

	struct Entry {

		Key    key    {};
		Value  value  {};
		State  state  {};
	};

	Entry entries_ [ Capacity ];

	enum class State : std::uint8_t {
		Empty,
		Occupied,
		Deleted
	};

public:
	explicit flat_map ()
		: {}

	std::size_t idx_ = key & MASK_;


};

} // namespace zerok
