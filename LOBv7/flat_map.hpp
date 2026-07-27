// flat map // 26.07.26 // ZeroK
// will replace std::unordered_map in orderbook for order_map_


#pragma once

namespace zerok {

template <
	typename Key,
	typename Value,
	std::size_t Capacity
>
class FlatMap {

private:

	static constexpr std::size_t  MASK  { Capacity - 1 };

	static_assert( Capacity > 0 && 
		      (Capacity & (Capacity - 1)) == 0,
		      "Capacity must be power of 2.\n" );


	enum class State : std::uint8_t {
		Empty,
		Occupied,
		Deleted
	};

	struct Entry {

		Key    key    {};
		Value  value  {};
		State  state  { State::Empty };
	};

	Entry entries_ [ Capacity ];

	std::size_t  size_  {};


public:

	// bool	     insert ( const Key&, const Value& );
	// Value*       find ( const Key& );
	// bool         erase ( const Key& );
	// bool 	     contains ( const Key& );
	// void	     clear();
	// bool	     empty() const noexcept;
	// std::size_t  size() const noexcept;


	bool insert ( const Key& key, const Value& value ) {

		auto idx  =  key & MASK;	// current bucket/idx

		for (auto i {0uz}; i < Capacity; ++i) {
		
			Entry& slot  =  entries_ [ idx ];

			if ( slot.state == State::Empty ||
			     slot.state == State::Deleted ) {

				slot.key    =  key;
				slot.value  =  value;
				slot.state  =  State::Occupied;

				++size_;
				return true;
			}

			// if duplicate key
			if ( slot.state == State::Occupied ||
			     slot.key == key ) return false;

			idx  =  (idx + 1) & MASK;	// advance bucket/idx
		}

		return false;	// table full
	}

};

} // namespace zerok

