// flat map v3 // 02.08.26 // ZeroK
/**Features v2:
 * control-byte meta data: stores the state of the spot for fast probing
 * eg: std::vector<char> ctrl_ - stores meta data
 * backward shift algo:
 * 	1. mark the erased slot as EMPTY - hole created
 * 	2. if next slot is EMPTY - stop
 * 	3. if next slot is FULL - compute next slot's ideal pos
 * 	4. if next slot is at ideal pos - stop
 * 	5. if next slot is not at ideal pos i.e. its current pos is b/w
 * 	   ideal pos and current pos - move the element into the current hole
 * 	6. advance the hole to shifted element's prev pos and repeat from step 2.
 *
 *
 **Features v3:
 * robin hood insert
 * elements far from ideal pos create collision clusters - thus increasing lookup time
 * robin hood solves this by reducing displacment, making lookup roughly O(1)
 * cache behavior - dense, predictable - better than liner probing that inserts at...
 * ...first free empty slot
 *
 * Algo:
 * 	1. "Take from the rich, give to the poor"
 * 	2. rich = elements with short displacement (close to ideal pos)
 * 	3. poor = elements with long displacment (far from ideal pos)
 * 	4. algo ensures to element is far from its ideal pos
 * */


#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
// #include <memory>


namespace zerok {

template <
	typename Key,		// need to be trivial
	typename Value,		// static_assert ahead
	std::size_t Capacity
>
class FlatMap {

private:

	static constexpr std::size_t  MASK  { Capacity - 1 };

	// states for control-byte meta data
	static constexpr std::uint8_t EMPTY  { 0 };
	static constexpr std::uint8_t FULL   { 1 };

	static_assert( Capacity > 0 && 
		      (Capacity & (Capacity - 1)) == 0,
		      "Capacity must be power of 2.\n" );

	static_assert( std::is_trivially_destructible_v<Key> );
	static_assert( std::is_trivially_destructible_v<Value> );


	// State enum removed - now in ctrl_
	struct Entry {

		Key    key    {};
		Value  value  {};
	};

	// Entry entries_ [ Capacity ];
	// std::unique_ptr<Entry[]> entries_; 
	std::vector<Entry> entries_;

	// control-byte meta data
	std::vector<std::uint8_t> ctrl_;

	[[ maybe_unused ]] char pad_ [ 64 ];
	alignas(64) std::size_t  size_  {};

	std::size_t  deleted_ {};	// debugging only


public:

	FlatMap() : entries_( Capacity ), ctrl_( Capacity ) {}
	// FlatMap() : entries_( std::make_unique<Entry[]>(Capacity) ) {}

	// bool	     insert ( const Key&, const Value& );
	// Value*       find ( const Key& );
	// bool         erase ( const Key& );
	// void	     clear();
	// bool	     empty() const noexcept;
	// std::size_t  size() const noexcept;
	// Entry*	probe ( const Key& );


	[[ nodiscard ]]
	bool insert ( Key key, Value value ) {	// need mutable temporaries - pass by val

		std::size_t idx  =  key & MASK;		// current bucket/idx
		std::size_t poor_dist  =  0uz;		// displacement of new ele

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_[ idx ];
			auto& state  =  ctrl_[ idx ];


			// avoid duplicate entry
			if ( state == FULL && 
			     slot.key == key ) 
				return false;

			// insert
			if ( state == EMPTY ) {

				slot.key    =  key;
				slot.value  =  value;

				state  =  FULL;
				++size_;
				return true;
			}

			// displacement of existing ele
			auto rich_ideal  =  slot.key & MASK;
			auto rich_dist   =  (idx - rich_ideal) & MASK;	
			
			// if existing element has smaller disp - swap
			// poor/new element replaces rich/existing element
			if ( poor_dist > rich_dist ) {
				
				std::swap( slot.key, key );
				std::swap( slot.value, value );
				std::swap( poor_dist, rich_dist );	
			}

			// advance idx
			idx  =  (idx + 1) & MASK;
			++poor_dist;
		}

		return false;	// table full
	}

	
	[[ nodiscard ]]
	Value*  find ( const Key& key ) {
		
		auto idx  =  key & MASK;

		// for (auto i {0uz}; i < Capacity; ++i) {
		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot = entries_[ idx ];
			auto& state  = ctrl_[ idx ];

			if ( state == EMPTY ) 
				return nullptr;

			if ( state == FULL &&
			     slot.key == key ) 
				return &slot.value;

			idx  =  (idx + 1) & MASK;
		}

		return nullptr;
	}	

	[[ nodiscard ]]
	const Value*  find ( const Key& key ) const {
		
		auto idx  =  key & MASK;

		for (auto i {0uz}; i < Capacity; ++i) {
			
			const Entry& slot = entries_[ idx ];

			if ( ctrl_[ idx ] == EMPTY ) 
				return nullptr;

			if ( ctrl_[ idx ] == FULL &&
			     slot.key == key ) 
				return &slot.value;

			idx  =  (idx + 1) & MASK;
		}

		return nullptr;
	}



	[[ nodiscard ]]
	bool  erase ( const Key& key ) {
		
		auto idx  =  key & MASK;

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot = entries_ [ idx ];
			auto& state  = ctrl_[ idx ];

			if ( state == FULL &&
			     slot.key == key ) {

				// found the target. erase it
				// update meta data - no need to erase payload
				// payload will be overwritten during insert()
				// slot  = {};
				state = EMPTY;	// mark the slot - hole 
				--size_;

				// now backfill the hole
				auto hole = idx;
				auto next = (hole + 1) & MASK;	// shift backward
								// to fill the hole

				// keep shifting till empty slot
				// or ideal pos comes
				while ( ctrl_[ next ] == FULL ) {

					Entry& next_slot  =  entries_[ next ];
					auto& next_state  =  ctrl_[ next ];

					if ( next_state == EMPTY )
						break;	// empty slot
							// stop shifting

					// ideal - natural pos of key, if 
					// there were no collisions
					auto ideal = next_slot.key & MASK;

					// check if element can shift backward
					// if ideal == next, at ideal pos - dont move
					// else, check if hole is b/w ideal pos and 
					// current pos/next
					/************************************************/
					bool hole_in_path { false };
					if ( ideal <= next ) 
						hole_in_path = ideal <= hole && 
								hole < next;
					else
						hole_in_path = ideal <= hole ||
								hole < next;

					if ( !hole_in_path ) break;
					/************************************************/


					// move next_slot into hole
					entries_[ hole ]  =  next_slot;
					ctrl_[ hole ]     =  FULL;
					ctrl_[ next ]     =  EMPTY;

					hole  =  next;
					next  =  (next + 1) & MASK;
				}

				return true;
			}

			idx  =  (idx + 1) & MASK;
		}

		return false;	// full table, key not found
	}



	void  clear () {
		
		for (auto& entry : entries_) { 
			entry = {};
		}

		size_  =  0;
	}


	[[ nodiscard ]]
	bool  empty () const noexcept {
		
		return size_ == 0;
	}


	[[ nodiscard ]]
	std::size_t size () const noexcept {
		
		return size_;
	}


	std::size_t deleted () { return deleted_; }

};

} // namespace zerok

