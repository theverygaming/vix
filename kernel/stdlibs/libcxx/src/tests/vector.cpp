#include <vector>
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

    volatile bool constructed = false;
    volatile bool destructed = false;
    volatile bool copied = false;
    volatile int value = 0;
};

TEST(std_vector_construct_destruct_1) {
    std::vector<testclass> vec;
    std::vector<testclass *> vec_ptr;

    for (size_t i = 0; i < 1000; i++) {
        vec.push_back(testclass(i));
    }

    // populate pointers
    vec_ptr.clear();
    for (size_t i = 0; i < vec.size(); i++) {
        vec_ptr.push_back(&vec[i]);
    }

    // check if all constructors + copy constructors were called and if data was copied correctly
    TEST_ASSERT(vec.size() == vec_ptr.size());
    for (size_t i = 0; i < vec_ptr.size(); i++) {
        TEST_ASSERT(vec_ptr[i]->constructed == true);
        TEST_ASSERT(vec_ptr[i]->destructed == false);
        TEST_ASSERT(vec_ptr[i]->copied == true);
        TEST_ASSERT(vec_ptr[i]->value == i);
        TEST_ASSERT(vec[i].constructed == true);
        TEST_ASSERT(vec[i].destructed == false);
        TEST_ASSERT(vec[i].copied == true);
        TEST_ASSERT(vec[i].value == i);
    }

    vec.clear();

    TEST_ASSERT(vec.size() == 0);

    // check if all destructors + constructors + copy constructors were called and if data was copied correctly
    for (size_t i = 0; i < vec_ptr.size(); i++) {
        TEST_ASSERT(vec_ptr[i]->constructed == true);
        TEST_ASSERT(vec_ptr[i]->destructed == true);
        TEST_ASSERT(vec_ptr[i]->copied == true);
        TEST_ASSERT(vec_ptr[i]->value == i);
    }

    return true;
}

TEST(std_vector_construct_destruct_2) {
    std::vector<testclass> vec;
    std::vector<testclass *> vec_ptr;

    for (size_t i = 0; i < 1000; i++) {
        vec.push_back(testclass(i));
    }

    // populate pointers
    vec_ptr.clear();
    for (size_t i = 0; i < vec.size(); i++) {
        vec_ptr.push_back(&vec[i]);
    }

    // check if all constructors + copy constructors were called and if data was copied correctly
    TEST_ASSERT(vec.size() == vec_ptr.size());
    for (size_t i = 0; i < vec_ptr.size(); i++) {
        TEST_ASSERT(vec_ptr[i]->constructed == true);
        TEST_ASSERT(vec_ptr[i]->destructed == false);
        TEST_ASSERT(vec_ptr[i]->copied == true);
        TEST_ASSERT(vec_ptr[i]->value == i);
        TEST_ASSERT(vec[i].constructed == true);
        TEST_ASSERT(vec[i].destructed == false);
        TEST_ASSERT(vec[i].copied == true);
        TEST_ASSERT(vec[i].value == i);
    }

    for (size_t i = 0; i < 1000; i++) {
        vec.pop_back();
    }
    TEST_ASSERT(vec.size() == 0);

    // check if all destructors + constructors + copy constructors were called and if data was copied correctly
    for (size_t i = 0; i < vec_ptr.size(); i++) {
        TEST_ASSERT(vec_ptr[i]->constructed == true);
        TEST_ASSERT(vec_ptr[i]->destructed == true);
        TEST_ASSERT(vec_ptr[i]->copied == true);
        TEST_ASSERT(vec_ptr[i]->value == i);
    }

    return true;
}

TEST(std_vector_int) {
    // fill vectors
    std::vector<int> vec1;
    TEST_ASSERT(vec1.size() == 0);
    for (size_t i = 0; i < 536; i++) {
        vec1.push_back(i);
    }
    TEST_ASSERT(vec1.size() == 536);
    std::vector<int> vec2;
    for (size_t i = 1234; i > 0; i--) {
        vec2.push_back(i);
    }
    TEST_ASSERT(vec2.size() == 1234);
    vec1.shrink_to_fit();

    vec2.reserve(1500);
    vec1.shrink_to_fit();

    // check their values
    TEST_ASSERT(vec1.size() == 536);
    for (size_t i = 0; i < vec1.size(); i++) {
        TEST_ASSERT(vec1[i] == i);
    }

    TEST_ASSERT(vec2.size() == 1234);
    int v = 1234;
    for (size_t i = 0; i < vec2.size(); i++) {
        TEST_ASSERT(vec2[i] == v--);
    }

    // test pop and erase
    vec2.pop_back();
    vec1.erase(132, 150);

    // check their values
    TEST_ASSERT(vec1.size() == (536 - 19));
    v = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        if (v == 132) {
            v = 151;
        }
        TEST_ASSERT(vec1[i] == v++);
    }

    TEST_ASSERT(vec2.size() == 1233);
    v = 1234;
    for (size_t i = 0; i < vec2.size(); i++) {
        TEST_ASSERT(vec2[i] == v--);
    }

    vec2.clear();
    vec2.push_back(15);
    vec2.push_back(182);

    // check their values
    TEST_ASSERT(vec1.size() == (536 - 19));
    v = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        if (v == 132) {
            v = 151;
        }
        TEST_ASSERT(vec1[i] == v++);
    }

    TEST_ASSERT(vec2.size() == 2);
    TEST_ASSERT(vec2[0] == 15);
    TEST_ASSERT(vec2[1] == 182);

    vec2 = vec1;

    // check their values
    TEST_ASSERT(vec1.size() == (536 - 19));
    v = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        if (v == 132) {
            v = 151;
        }
        TEST_ASSERT(vec1[i] == v++);
    }
    TEST_ASSERT(vec2.size() == (536 - 19));
    v = 0;
    for (size_t i = 0; i < vec2.size(); i++) {
        if (v == 132) {
            v = 151;
        }
        TEST_ASSERT(vec2[i] == v++);
    }

    return true;
}
