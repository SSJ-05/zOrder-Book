// Unit tests for flat_map // 29.07.26 // ZeroK

#include <cassert>
#include <iostream>
#include <string>
#include <cstdint>

// #include "flat_map_v1.hpp"
// #include "flat_map_v2.hpp"
#include "flat_map_v3.hpp"

// Custom key type to test non‑integral keys
struct TestKey {
    uint64_t id;
    uint64_t seq;

    // Needed for hashing (simple XOR)
    operator uint64_t() const { return id ^ seq; }

    bool operator==(const TestKey& other) const {
        return id == other.id && seq == other.seq;
    }
};

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


void test_insert() {
    zerok::FlatMap<uint64_t, int, 8> map;

    bool ok = map.insert(42, 100);
    ASSERT(ok);
    ASSERT_EQ(map.size(), 1);

    auto* val = map.find(42);
    ASSERT_NOTNULL(val);
    ASSERT_EQ(*val, 100);
}


void test_duplicate_insert() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(42, 100);
    bool ok = map.insert(42, 200);  // Should fail (duplicate key)
    ASSERT(!ok);
    ASSERT_EQ(map.size(), 1);

    auto* val = map.find(42);
    ASSERT_NOTNULL(val);
    ASSERT_EQ(*val, 100);  // Value unchanged
}


// void test_find_existing() {
//     zerok::FlatMap<uint64_t, std::string, 8> map;
//
//     map.insert(1, "one");
//     map.insert(2, "two");
//     map.insert(3, "three");
//
//     auto* val = map.find(2);
//     ASSERT_NOTNULL(val);
//     ASSERT_EQ(*val, "two");
// }


void test_find_missing() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(10, 100);
    map.insert(20, 200);

    auto* val = map.find(30);
    ASSERT_NULL(val);

    // Non‑const version
    auto* val2 = map.find(40);
    ASSERT_NULL(val2);
}


void test_erase_existing() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(5, 555);
    ASSERT_EQ(map.size(), 1);

    bool ok = map.erase(5);
    ASSERT(ok);
    ASSERT_EQ(map.size(), 0);

    auto* val = map.find(5);
    ASSERT_NULL(val);
}


void test_erase_missing() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(1, 100);
    bool ok = map.erase(99);  // Key doesn't exist
    ASSERT(!ok);
    ASSERT_EQ(map.size(), 1);
}


void test_erase_then_find() {
    zerok::FlatMap<uint64_t, int, 8> map;

    map.insert(7, 777);
    map.insert(8, 888);

    map.erase(7);
    auto* val = map.find(7);
    ASSERT_NULL(val);

    auto* val2 = map.find(8);
    ASSERT_NOTNULL(val2);
    ASSERT_EQ(*val2, 888);
}


void test_clear() {
    zerok::FlatMap<uint64_t, int, 16> map;

    for (int i = 0; i < 10; ++i) {
        map.insert(i, i * 10);
    }
    ASSERT_EQ(map.size(), 10);

    map.clear();
    ASSERT_EQ(map.size(), 0);
    ASSERT(map.empty());

    auto* val = map.find(5);
    ASSERT_NULL(val);
}



void test_table_full() {
    constexpr size_t CAP = 4;
    zerok::FlatMap<uint64_t, int, CAP> map;

    // Fill the table (CAP entries)
    for (size_t i = 0; i < CAP; ++i) {
        bool ok = map.insert(i, i * 10);
        ASSERT(ok);
    }
    ASSERT_EQ(map.size(), CAP);

    // Next insert should fail
    bool ok = map.insert(CAP, 999);
    ASSERT(!ok);
    ASSERT_EQ(map.size(), CAP);
}



void test_wraparound() {
    constexpr size_t CAP = 4;
    zerok::FlatMap<uint64_t, int, CAP> map;

    // Force keys that collide and wrap
    // Keys: 0, 4, 8, 12 (all hash to index 0)
    // Using CAP=4 and keys multiple of 4
    for (size_t i = 0; i < CAP; ++i) {
        uint64_t key = i * 4;
        bool ok = map.insert(key, key * 10);
        ASSERT(ok);
    }

    // Now find a key that required wrap
    auto* val = map.find(12);
    ASSERT_NOTNULL(val);
    ASSERT_EQ(*val, 120);

    // Erase a key that required wrap
    bool ok = map.erase(8);
    ASSERT(ok);
    ASSERT_EQ(map.size(), CAP - 1);

    // Re‑insert should find the deleted slot (wraparound test)
    ok = map.insert(16, 160);
    ASSERT(ok);
    ASSERT_EQ(map.size(), CAP);
}


// void test_custom_key() {
//     zerok::FlatMap<TestKey, std::string, 8> map;
//
//     TestKey k1{100, 1};
//     TestKey k2{100, 2};
//
//     map.insert(k1, "first");
//     map.insert(k2, "second");
//
//     auto* v1 = map.find(k1);
//     ASSERT_NOTNULL(v1);
//     ASSERT_EQ(*v1, "first");
//
//     TestKey missing{999, 0};
//     auto* v2 = map.find(missing);
//     ASSERT_NULL(v2);
// }


int main() {
    std::cout << "\n=== FlatMap Unit Tests ===\n\n";

    TEST(insert);
    TEST(duplicate_insert);
    // TEST(find_existing);
    TEST(find_missing);
    TEST(erase_existing);
    TEST(erase_missing);
    TEST(erase_then_find);
    TEST(clear);
    TEST(table_full);
    TEST(wraparound);
    // TEST(custom_key);

    std::cout << "\nAll tests PASSED.\n";
    return EXIT_SUCCESS;
}
