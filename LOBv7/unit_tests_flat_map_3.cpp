// unit tests for flatmap v4 edge cases // 06.08.26 // ZeroK

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "flat_map_v4.hpp"


static void test_hash_31() {
    constexpr std::size_t CAP = 1024;
    zerok::FlatMap<uint64_t, uint64_t, CAP> map;

    // Key chosen so that (key & MASK) == 31
    // MASK = 1023, so key = 31 works directly
    uint64_t key = 31;
    
    bool ok = map.insert(key, 100);
    assert(ok && "Insert at hash 31 failed");
    
    uint64_t* val = map.find(key);
    assert(val != nullptr && "Find at hash 31 failed");
    assert(*val == 100 && "Wrong value at hash 31");
    
    // Verify the entry is at the correct physical slot
    // hash 31 is at offset 31 within group 0 (group starts at 0)
    // So the control byte at index 31 should match fingerprints(31)
    assert(map.size() == 1 && "Size should be 1");
    
    std::printf("PASS: test_hash_31\n");
}


static void test_hash_32() {
    constexpr std::size_t CAP = 1024;
    zerok::FlatMap<uint64_t, uint64_t, CAP> map;

    // Key = 32, hash = 32 & 1023 = 32
    // This is at the start of group 1 (group starts at 32)
    uint64_t key = 32;
    
    bool ok = map.insert(key, 200);
    assert(ok && "Insert at hash 32 failed");
    
    uint64_t* val = map.find(key);
    assert(val != nullptr && "Find at hash 32 failed");
    assert(*val == 200 && "Wrong value at hash 32");
    
    // Insert another key that also hashes to 32
    uint64_t key2 = 32 + CAP;  // hash = (32 + 1024) & 1023 = 32
    ok = map.insert(key2, 300);
    assert(ok && "Insert at colliding hash 32 failed");
    
    val = map.find(key2);
    assert(val != nullptr && "Find colliding key failed");
    assert(*val == 300 && "Wrong value for colliding key");
    
    // Both keys should be findable
    val = map.find(key);
    assert(val != nullptr && "First key lost after collision");
    assert(*val == 200 && "First key value corrupted");
    
    assert(map.size() == 2 && "Size should be 2");
    
    std::printf("PASS: test_hash_32\n");
}


static void test_hash_capacity_minus_1() {
    constexpr std::size_t CAP = 1024;
    zerok::FlatMap<uint64_t, uint64_t, CAP> map;

    // Key = 1023, hash = 1023 & 1023 = 1023
    // This is at offset 31 within group starting at 992 (Capacity - 32)
    // SIMD load at ctrl_[992] reads bytes [992..1023], last byte at 1023
    // This is within Capacity, and within Capacity + GROUP_SIZE
    uint64_t key = CAP - 1;  // 1023
    
    bool ok = map.insert(key, 999);
    assert(ok && "Insert at Capacity-1 failed");
    
    uint64_t* val = map.find(key);
    assert(val != nullptr && "Find at Capacity-1 failed");
    assert(*val == 999 && "Wrong value at Capacity-1");
    
    // Insert a key that wraps around: hash = 0
    uint64_t key2 = CAP;  // hash = 1024 & 1023 = 0
    ok = map.insert(key2, 111);
    assert(ok && "Insert at hash 0 failed");
    
    val = map.find(key2);
    assert(val != nullptr && "Find at hash 0 failed");
    assert(*val == 111 && "Wrong value at hash 0");
    
    // Both still findable
    val = map.find(key);
    assert(val != nullptr && "Key at Capacity-1 lost");
    assert(*val == 999 && "Key at Capacity-1 corrupted");
    
    assert(map.size() == 2 && "Size should be 2");
    
    std::printf("PASS: test_hash_capacity_minus_1\n");
}



static void test_wraparound_insert() {
    constexpr std::size_t CAP = 1024;
    zerok::FlatMap<uint64_t, uint64_t, CAP> map;

    // Fill slots from 1020 to 1023 (last 4 slots before wraparound)
    // Use keys that hash exactly to these positions
    // key = position works because MASK = 1023 and position < 1024
    map.insert(1020, 1);
    map.insert(1021, 2);
    map.insert(1022, 3);
    map.insert(1023, 4);
    
    // Now insert a key that hashes to 1022 (already occupied)
    // It should probe: 1022 (occupied), 1023 (occupied), 0 (free after wrap)
    uint64_t key = 1022;  // collides with existing key at 1022
    // We need a DIFFERENT key that hashes to the same slot
    uint64_t key_collide = 1022 + CAP;  // hash = (1022 + 1024) & 1023 = 1022
    
    bool ok = map.insert(key_collide, 999);
    assert(ok && "Wraparound insert failed");
    
    uint64_t* val = map.find(key_collide);
    assert(val != nullptr && "Find wraparound key failed");
    assert(*val == 999 && "Wrong value for wraparound key");
    
    // The new key should be at slot 0 (after wraparound), or displaced elsewhere
    // Verify all original keys still findable
    assert(map.find(1020) && *map.find(1020) == 1);
    assert(map.find(1021) && *map.find(1021) == 2);
    assert(map.find(1022) && *map.find(1022) == 3);
    assert(map.find(1023) && *map.find(1023) == 4);
    
    assert(map.size() == 5 && "Size should be 5");
    
    std::printf("PASS: test_wraparound_insert\n");
}



static void test_wraparound_erase() {
    constexpr std::size_t CAP = 1024;
    zerok::FlatMap<uint64_t, uint64_t, CAP> map;

    // Setup: fill last 2 slots and first 2 slots to create a cluster
    // that wraps around the end of the array
    map.insert(1022, 10);           // hash 1022
    map.insert(1023, 20);           // hash 1023
    uint64_t key_wrap = 1022 + CAP; // hash 1022, probes to 0 after wrap
    map.insert(key_wrap, 30);       // should land at slot 0 (or displaced)
    map.insert(1 + CAP, 40);        // hash 1, probes near slot 1
    
    assert(map.size() == 4 && "Setup size wrong");
    
    // Verify all inserted
    assert(map.find(1022) && *map.find(1022) == 10);
    assert(map.find(1023) && *map.find(1023) == 20);
    assert(map.find(key_wrap) && *map.find(key_wrap) == 30);
    assert(map.find(1 + CAP) && *map.find(1 + CAP) == 40);
    
    // Erase the element at 1022 — this creates a hole at the boundary
    bool erased = map.erase(1022);
    assert(erased && "Erase at boundary failed");
    assert(map.size() == 3 && "Size should be 3 after erase");
    
    // Erased key should not be found
    assert(map.find(1022) == nullptr && "Erased key still findable");
    
    // Backfill should have shifted elements if needed
    // All remaining keys should still be findable
    assert(map.find(1023) && *map.find(1023) == 20);
    assert(map.find(key_wrap) && *map.find(key_wrap) == 30);
    assert(map.find(1 + CAP) && *map.find(1 + CAP) == 40);
    
    // Now erase the element at 1023 (next to the previous hole, at the end)
    erased = map.erase(1023);
    assert(erased && "Erase at 1023 failed");
    assert(map.size() == 2 && "Size should be 2 after second erase");
    assert(map.find(1023) == nullptr && "Erased key 1023 still findable");
    
    // Remaining keys still findable
    assert(map.find(key_wrap) && *map.find(key_wrap) == 30);
    assert(map.find(1 + CAP) && *map.find(1 + CAP) == 40);
    
    // Re-insert at the erased positions to verify slots are reusable
    bool re_ok = map.insert(1022, 999);
    assert(re_ok && "Re-insert at 1022 failed");
    assert(map.find(1022) && *map.find(1022) == 999);
    
    re_ok = map.insert(1023, 888);
    assert(re_ok && "Re-insert at 1023 failed");
    assert(map.find(1023) && *map.find(1023) == 888);
    
    assert(map.size() == 4 && "Final size should be 4");
    
    std::printf("PASS: test_wraparound_erase\n");
}



int main() {
    std::printf("=== FlatMap V4 Edge Case Tests ===\n\n");
    
    test_hash_31();
    test_hash_32();
    test_hash_capacity_minus_1();
    test_wraparound_insert();
    test_wraparound_erase();
    
    std::printf("\n=== All 5 tests passed ===\n");
    return EXIT_SUCCESS;
}
