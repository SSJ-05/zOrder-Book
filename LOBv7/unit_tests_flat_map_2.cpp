// unit test for erase() to test edge cases // 31.07.26 // ZeroK

#include <cassert>
#include <iostream>
#include <cstdint>
#include "flat_map_v2.hpp"

#define TEST(name) \
    std::cout << "[TEST] " #name << "... "; \
    test_##name(); \
    std::cout << "PASSED\n"

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAILED at line " << __LINE__ << ": " << #cond << "\n"; \
            std::abort(); \
        } \
    } while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NEQ(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(ptr) ASSERT((ptr) == nullptr)
#define ASSERT_NOTNULL(ptr) ASSERT((ptr) != nullptr)

void test_erase_simple_shift() {
    zerok::FlatMap<uint64_t, int, 8> map;

    // Keys that collide (all hash to slot 0)
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);

    ASSERT_EQ(map.size(), 3);

    // Erase middle element (0)
    bool ok = map.erase(0);
    ASSERT(ok);
    ASSERT_EQ(map.size(), 2);

    // Find remaining keys
    auto* v1 = map.find(8);
    ASSERT_NOTNULL(v1);
    ASSERT_EQ(*v1, 200);

    auto* v2 = map.find(16);
    ASSERT_NOTNULL(v2);
    ASSERT_EQ(*v2, 300);
}


void test_erase_then_insert() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(5, 100);
    map.insert(13, 200);
    map.insert(21, 300);

    map.erase(5);

    // Insert new key that collides
    map.insert(29, 400);

    auto* v1 = map.find(13);
    ASSERT_NOTNULL(v1);
    ASSERT_EQ(*v1, 200);

    auto* v2 = map.find(21);
    ASSERT_NOTNULL(v2);
    ASSERT_EQ(*v2, 300);

    auto* v3 = map.find(29);
    ASSERT_NOTNULL(v3);
    ASSERT_EQ(*v3, 400);
}


void test_erase_middle_shift() {
    zerok::FlatMap<uint64_t, int, 8> map;

    // Keys: 0, 8, 16, 24 (all hash to slot 0)
    for (int i = 0; i < 4; ++i) {
        map.insert(i * 8, i * 100);
    }
    ASSERT_EQ(map.size(), 4);

    // Erase 8 (middle)
    map.erase(8);
    ASSERT_EQ(map.size(), 3);

    // Insert new key 32 (should find the hole)
    map.insert(32, 500);

    auto* v = map.find(32);
    ASSERT_NOTNULL(v);
    ASSERT_EQ(*v, 500);
}


void test_erase_wraparound() {
    zerok::FlatMap<uint64_t, int, 8> map;

    // Fill table with keys that collide
    for (int i = 0; i < 8; ++i) {
        map.insert(i * 8, i * 10);
    }
    ASSERT_EQ(map.size(), 8);

    // Erase first element (0) – this forces wraparound shift
    map.erase(0);
    ASSERT_EQ(map.size(), 7);

    // Erase key at the end (56) – also triggers wraparound
    map.erase(56);
    ASSERT_EQ(map.size(), 6);

    // Verify remaining keys exist
    for (int i = 1; i < 7; ++i) {
        auto* v = map.find(i * 8);
        ASSERT_NOTNULL(v);
        ASSERT_EQ(*v, i * 10);
    }
}


void test_erase_non_existent() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(10, 100);
    map.insert(20, 200);

    bool ok = map.erase(99);
    ASSERT(!ok);
    ASSERT_EQ(map.size(), 2);
}


void test_erase_all() {
    zerok::FlatMap<uint64_t, int, 8> map;

    for (int i = 0; i < 4; ++i) {
        map.insert(i * 8, i * 100);
    }
    ASSERT_EQ(map.size(), 4);

    for (int i = 0; i < 4; ++i) {
        bool ok = map.erase(i * 8);
        ASSERT(ok);
    }
    ASSERT_EQ(map.size(), 0);

    // Verify all erased
    for (int i = 0; i < 4; ++i) {
        auto* v = map.find(i * 8);
        ASSERT_NULL(v);
    }
}



void test_erase_reinsert() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(100, 1);
    map.insert(200, 2);

    map.erase(100);
    auto* v = map.find(100);
    ASSERT_NULL(v);

    // Reinsert at same key
    bool ok = map.insert(100, 999);
    ASSERT(ok);

    auto* v2 = map.find(100);
    ASSERT_NOTNULL(v2);
    ASSERT_EQ(*v2, 999);
}



void test_erase_at_capacity() {
    zerok::FlatMap<uint64_t, int, 8> map;

    // Fill table completely
    for (int i = 0; i < 8; ++i) {
        map.insert(i * 8, i * 10);
    }
    ASSERT_EQ(map.size(), 8);

    // Erase all keys
    for (int i = 0; i < 8; ++i) {
        map.erase(i * 8);
    }
    ASSERT_EQ(map.size(), 0);

    // Reinsert
    map.insert(0, 999);
    auto* v = map.find(0);
    ASSERT_NOTNULL(v);
    ASSERT_EQ(*v, 999);
}



void test_erase_no_tombstones() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);

    map.erase(8);

    // The hole should be filled by shifting 16 backward
    // Insert new key 24 – should go to slot 1 (the former hole)
    map.insert(24, 400);

    auto* v = map.find(24);
    ASSERT_NOTNULL(v);
    ASSERT_EQ(*v, 400);

    // Ensure 16 is still findable
    auto* v2 = map.find(16);
    ASSERT_NOTNULL(v2);
    ASSERT_EQ(*v2, 300);
}


int main() {
    std::cout << "\n=== FlatMap Erase Edge Cases ===\n\n";

    TEST(erase_simple_shift);
    TEST(erase_then_insert);
    TEST(erase_middle_shift);
    TEST(erase_wraparound);
    TEST(erase_non_existent);
    TEST(erase_all);
    TEST(erase_reinsert);
    TEST(erase_at_capacity);
    TEST(erase_no_tombstones);

    std::cout << "\nAll tests PASSED.\n";
    return EXIT_SUCCESS;
}
