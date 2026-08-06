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
 * NOTE: overallocate vectors to avoid UB after avx load and prefetch
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

	static constexpr std::size_t   MASK_   { Capacity - 1 };

	// states for control-byte meta data
	static constexpr std::uint8_t  EMPTY_       { 0x80 };
	static constexpr std::uint8_t  GROUP_SIZE_  { 32 };

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
			_mm256_set1_epi8( static_cast<char>( EMPTY_ ) );

		// compare
		const __m256i cmp =
			_mm256_cmpeq_epi8( group, target );

		// convert to bitmask
		return  static_cast<std::uint32_t>( 
				_mm256_movemask_epi8( cmp ) );
	}


public:

	// overallocate to avoid UB in avx load
	FlatMap() : entries_( Capacity ), 
		    ctrl_( Capacity + GROUP_SIZE_ ) 
	{
		std::fill( ctrl_.begin(), ctrl_.begin() + Capacity, EMPTY_ );
		std::fill( ctrl_.begin() + Capacity, ctrl_.end(), EMPTY_ );
	}
	

	// helper func
	inline std::uint8_t  fingerprints ( Key key ) {
		
		return static_cast<std::uint8_t>( key & 0x7F );
	}

	inline void sync_mirror ( std::size_t idx ) noexcept {
		
		if ( idx < GROUP_SIZE_ )
			ctrl_[ Capacity + idx ] = ctrl_[ idx ];
	}

	[[ nodiscard ]]
	bool insert ( Key key, Value value ) {	// need mutable temporaries - pass by val

		std::size_t idx  =  key & MASK_;	// current bucket/idx
		std::size_t poor_dist  =  0uz;		// displacement of new ele

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_[ idx ];
			auto& state  =  ctrl_[ idx ];
			auto  fp     =  fingerprints( key );

			// avoid duplicate entry
			if ( state == fp && slot.key == key ) 
				return false;

			// insert
			if ( state == EMPTY_ ) {

				slot.key    =  key;
				slot.value  =  value;

				state  =  fp;
				sync_mirror( idx );
				++size_;
				return true;
			}

			// displacement of existing ele
			auto rich_ideal  =  slot.key & MASK_;
			auto rich_dist   =  (idx - rich_ideal) & MASK_;	
			
			// if existing element has smaller disp - swap
			// poor/new element replaces rich/existing element
			if ( poor_dist > rich_dist ) {
				
				std::swap( slot.key,   key );
				std::swap( slot.value, value );
				std::swap( poor_dist,  rich_dist );	

				state = fingerprints( slot.key );
				sync_mirror( idx );
			}

			// advance idx
			idx  =  (idx + 1) & MASK_;
			++poor_dist;
		}

		return false;	// table full
	}

	
	[[ nodiscard ]]
	Value*  find ( const Key& key ) {
		
		const std::size_t ideal  =  key & MASK_;
		const std::uint8_t fp    =  fingerprints( key );

		// align to group start
		std::size_t group_idx  =  ideal & ~(GROUP_SIZE_ - 1);

		// track probe dist while advancing thru groups
		std::size_t probe_dist  {};

		for (auto _ {0uz}; _ < Capacity; _ += GROUP_SIZE_) {

			// prefetch entries_ (more expensive than ctrl_)
			std::size_t next_group =
				(group_idx + GROUP_SIZE_) & MASK_;

			__builtin_prefetch( &entries_[ next_group ],	// addr to prefetch
					    0, 				// read - 0, write - 1
					    1 );			// locality - L3 cache 

			// load 32 control bytes and match fingerprints
			auto mask  =  match_group( group_idx, fp );

			while ( mask ) {
				
				const int offset  =  __builtin_ctz( mask );
				const int idx     =  group_idx + offset;

				if ( entries_[ idx ].key == key )
					return &entries_[ idx ].value;

				mask  &=  (mask - 1); 
			}

			// check if we passed any EMPTY slots
			// need to preserve robinhood invariant
			auto empty_mask  =  match_empty( group_idx );

			if ( empty_mask ) {
				
				std::size_t first_empty =
					group_idx + __builtin_ctz( empty_mask );

				std::size_t empty_dist =
					(first_empty - ideal) & MASK_;

				// probe_dist never exceeds Capacity
				// as it is bounded by outer loop
				if ( empty_dist >= probe_dist )
					return nullptr;
			}

			group_idx  =  (group_idx + GROUP_SIZE_) & MASK_;
			probe_dist += GROUP_SIZE_;
		}

		return nullptr;		// entry not found
	}	

	// [[ nodiscard ]]
	// const Value*  find ( const Key& key ) const {
	//
	// 	auto idx  =  key & MASK;
	//
	// 	// for (auto i {0uz}; i < Capacity; ++i) {
	// 	for (auto _ {Capacity}; _-- > 0;) {
	//
	// 		const Entry& slot = entries_[ idx ];
	//
	// 		if ( ctrl_[ idx ] == EMPTY ) 
	// 			return nullptr;
	//
	// 		if ( ctrl_[ idx ] == fingerprints( key ) &&
	// 		     slot.key == key ) 
	// 			return &slot.value;
	//
	// 		idx  =  (idx + 1) & MASK;
	// 	}
	//
	// 	return nullptr;
	// }



	[[ nodiscard ]]
	bool  erase ( const Key& key ) {
		
		std::size_t idx  =  key & MASK_;

		for (auto _ {Capacity}; _-- > 0;) {
			
			Entry& slot  =  entries_ [ idx ];
			auto& state  =  ctrl_[ idx ];

			if ( state != EMPTY_ && slot.key == key ) {

				// found the target. erase it
				// update meta data - no need to erase payload
				// payload will be overwritten during insert()
				// slot  = {};
				state = EMPTY_;	// mark the slot - hole 
				sync_mirror( idx );
				--size_;

				// now backfill the hole
				auto hole = idx;
				auto next = (hole + 1) & MASK_;	// shift backward
								// to fill the hole

				// keep shifting till empty slot
				// or till ideal pos comes
				while ( ctrl_[ next ] != EMPTY_ ) {

					Entry& next_slot  =  entries_[ next ];

					// ideal - natural pos of key, if 
					// there were no collisions
					auto ideal  =  next_slot.key & MASK_;
				
					// **Robinhood invariant
					// after shifting, displacment must not
					// exceed threshold
					auto dist   =  (next - ideal) & MASK_;

					// if ele at ideal pos - stop shifting
					if ( dist == 0 ) break;

					// check if hole in the probe path
					auto hole_dist  =  (hole - ideal) & MASK_;
					if ( hole_dist > dist ) break;
					/************************************************/

					// shift backward
					// move next_slot into hole
					entries_[ hole ]  =  next_slot;
					
					ctrl_[ hole ]     =  ctrl_[ next ];
					sync_mirror( hole );
					
					ctrl_[ next ]     =  EMPTY_;
					sync_mirror( next );

					hole  =  next;
					next  =  (next + 1) & MASK_;
				}

				return true;
			}

			idx  =  (idx + 1) & MASK_;
		}

		return false;	// full table, key not found
	}



	void  clear () {
		
		for (auto& entry : entries_) { 
			entry = {};
		}

		std::fill( ctrl_.begin(),
			   ctrl_.begin() + Capacity + GROUP_SIZE_,
			   EMPTY_ );

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

