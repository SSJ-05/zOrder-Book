// flat map v4 // 03.08.26 // ZeroK
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
 *
 *
 **Features v4:
 * Fingerprints
 * SIMD group probing
 * Prefetch
 * */


#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <immintrin.h>


namespace zerok {

template <
	typename Key,		// need to be trivial
	typename Value,		// static_assert ahead
	std::size_t Capacity
>
class FlatMap {

private:

	static constexpr std::size_t   MASK      { Capacity - 1 };

	// states for control-byte meta data
	static constexpr std::uint8_t  EMPTY     { 0x80 };
	static constexpr std::uint8_t  GRP_SIZE  { 32 };

	static_assert( Capacity > 0 && 
		      (Capacity & (Capacity - 1)) == 0,
		      "Capacity must be power of 2.\n" );

	static_assert( std::is_trivially_copyable_v<Key> );
	static_assert( std::is_trivially_copyable_v<Value> );


	// State enum removed - now in ctrl_
	alignas(64) struct Entry {

		Key    key    {};
		Value  value  {};
	};

	// Payload
	[[ maybe_unused ]] char pad_0 [ 64 ];
	std::vector<Entry> entries_;

	// control-byte meta data
	[[ maybe_unused ]] char pad_1 [ 64 ];
	std::vector<std::uint8_t> ctrl_;

	[[ maybe_unused ]] char pad_2 [ 64 ];
	alignas(64) 
	std::size_t  size_  {};


	[[ nodiscard ]]
	inline std::uint32_t  match_group ( std::size_t idx, 
					    std::uint8_t fp ) const noexcept {

		// load 32 control bytes into 1 avx register
		const __m256i group  =  
			_mm256_loadu_si256(
				reinterpret_cast<const __m256i*>( &ctrl_[ idx ] ) ); 

		// broadcast fingerprint
		const __m256i target  =
			_mm256_set1_epi8(
				static_cast<char>( fp ) );

		// compare
		const __m256i cmp  =
			_mm256_cmpeq_epi8( group, target );


		// convert to bitmask
		return  static_cast<std::uint32_t>(
				_mm256_movemask_epi8( cmp ) );
	}


	// return bitmask of all EMPTY control bytes in a group
	[[ nodiscard ]]
	inline std::uint32_t  match_empty ( std::size_t idx ) const noexcept {
	
		// load the grp in avx register
		const __m256i group =
			_mm256_loadu_si256(
				reinterpret_cast<const __m256i*>( &ctrl_[ idx ] ) );

		// broadcast
		const __m256i target =
			_mm256_set1_epi8( static_cast<char>( EMPTY ) );

		// compare
		const __m256i cmp =
			_mm256_cmpeq_epi8( group, target );

		// convert to bitmask
		return  static_cast<std::uint32_t>( 
				_mm256_movemask_epi8( cmp ) );
	}


public:

	FlatMap() : entries_( Capacity ), ctrl_( Capacity ) {}
	

	// helper func
	inline std::uint8_t  fingerprints ( Key key ) {
		
		return static_cast<std::uint8_t>( key & 0x7F );
	}



	[[ nodiscard ]]
	bool insert ( Key key, Value value ) {	// need mutable temporaries - pass by val

		std::size_t idx  =  key & MASK;		// current bucket/idx
		std::size_t poor_dist  =  0uz;		// displacement of new ele

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_[ idx ];
			auto& state  =  ctrl_[ idx ];
			auto  fp     =  fingerprints( key );

			// avoid duplicate entry
			if ( state == fp && slot.key == key ) 
				return false;

			// insert
			if ( state == EMPTY ) {

				slot.key    =  key;
				slot.value  =  value;

				state  =  fp;
				++size_;
				return true;
			}

			// displacement of existing ele
			auto rich_ideal  =  slot.key & MASK;
			auto rich_dist   =  (idx - rich_ideal) & MASK;	
			
			// if existing element has smaller disp - swap
			// poor/new element replaces rich/existing element
			if ( poor_dist > rich_dist ) {
				
				std::swap( slot.key,   key );
				std::swap( slot.value, value );
				std::swap( poor_dist,  rich_dist );	
			}

			// advance idx
			idx  =  (idx + 1) & MASK;
			++poor_dist;
		}

		return false;	// table full
	}

	
	[[ nodiscard ]]
	Value*  find ( const Key& key ) {
		
		std::size_t idx  =  key & MASK;
		std::uint8_t fp  =  fingerprints( key );

		// for (auto i {0uz}; i < Capacity; ++i) {
		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_[ idx ];
			auto state   =  ctrl_[ idx ];

			if ( state == EMPTY ) 
				return nullptr;

			if ( state == fp && slot.key == key ) 
				return &slot.value;

			idx  =  (idx + 1) & MASK;

			// prefetch entries_ (more expensive than ctrl_)
			__builtin_prefetch( &entries_[ next_grp ],	// addr to prefetch
					    0, 				// read - 0, write - 1
					    1 );			// locality - L3 cache 
		}

		return nullptr;
	}	

	[[ nodiscard ]]
	const Value*  find ( const Key& key ) const {
		
		auto idx  =  key & MASK;

		// for (auto i {0uz}; i < Capacity; ++i) {
		for (auto _ {Capacity}; _-- > 0;) {
			
			const Entry& slot = entries_[ idx ];

			if ( ctrl_[ idx ] == EMPTY ) 
				return nullptr;

			if ( ctrl_[ idx ] == fingerprints( key ) &&
			     slot.key == key ) 
				return &slot.value;

			idx  =  (idx + 1) & MASK;
		}

		return nullptr;
	}



	[[ nodiscard ]]
	bool  erase ( const Key& key ) {
		
		std::size_t idx  =  key & MASK;

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_ [ idx ];
			auto& state  =  ctrl_[ idx ];

			if ( state != EMPTY && slot.key == key ) {

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
				// or till ideal pos comes
				while ( ctrl_[ next ] != EMPTY ) {

					Entry& next_slot  =  entries_[ next ];

					// ideal - natural pos of key, if 
					// there were no collisions
					auto ideal  =  next_slot.key & MASK;
				
					// **Robinhood invariant
					// after shifting, displacment must not...
					// ...exceed threshold
					// check if hole is in probe path and disp is...
					// ...below threshold
					auto dist   =  (next - ideal) & MASK;

					// if ele at ideal pos - stop shifting
					if ( dist == 0 ) break;
					/************************************************/

					// shift backward
					// move next_slot into hole
					entries_[ hole ]  =  next_slot;
					ctrl_[ hole ]     =  fingerprints( key );
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

		std::fill( ctrl_.begin(),
			   ctrl_.end(),
			   EMPTY );

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

};

} // namespace zerok

