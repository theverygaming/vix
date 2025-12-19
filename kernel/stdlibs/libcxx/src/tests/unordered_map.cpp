#include <unordered_map>
#include <vix/test.h>
#include <vix/types.h>

class testclass {
public:
    testclass(int _value) {
        constructed = true;
        value = _value;
    }

    ~testclass() {
        destructed = true;
    }

    testclass(const testclass &obj) {
        constructed = true;
        destructed = false;
        copied = true;
        value = obj.value;
    }

    testclass &operator=(const testclass &tc) {
        value = tc.value;
        return *this;
    }

    volatile bool constructed = false;
    volatile bool destructed = false;
    volatile bool copied = false;
    volatile int value = 0;
};

struct testclass_factory {
    testclass operator()() const {
        return testclass(-1234);
    }
};

TEST(std_unordered_map_int) {
    std::unordered_map<int, testclass, testclass_factory> map1;

    // insertion
    TEST_ASSERT(!map1.contains(1));
    map1[1];
    TEST_ASSERT(map1.contains(1));
    TEST_ASSERT(map1[1].value == -1234);

    TEST_ASSERT(!map1.contains(2));
    map1[2] = testclass(25);
    TEST_ASSERT(map1.contains(2));
    TEST_ASSERT(map1[2].value == 25);

    TEST_ASSERT(!map1.contains(3));
    map1.insert(3, testclass(26));
    TEST_ASSERT(map1.contains(3));
    TEST_ASSERT(map1[3].value == 26);

    // replace
    TEST_ASSERT(map1[1].value == -1234);
    map1[1] = testclass(5);
    TEST_ASSERT(map1[1].value == 5);

    TEST_ASSERT(map1[2].value == 25);
    map1[2] = testclass(6);
    TEST_ASSERT(map1[2].value == 6);

    // replace pointers/references
    auto ptr = &map1[4];
    map1[4] = testclass(7);
    TEST_ASSERT(ptr == &map1[4]);

    ptr = &map1[3];
    auto ptr2 = &map1.insert(3, testclass(27));
    TEST_ASSERT(ptr == &map1[3]);
    TEST_ASSERT(ptr == ptr2);
    TEST_ASSERT(map1[3].value == 27);

    // erase
    TEST_ASSERT(map1.contains(1));
    map1.erase(1);
    TEST_ASSERT(!map1.contains(1));

    TEST_ASSERT(map1.contains(2));
    map1.erase(2);
    TEST_ASSERT(!map1.contains(2));

    TEST_ASSERT(map1.contains(3));
    map1.erase(3);
    TEST_ASSERT(!map1.contains(3));

    TEST_ASSERT(map1.contains(4));
    map1.erase(4);
    TEST_ASSERT(!map1.contains(4));

    return true;
}
