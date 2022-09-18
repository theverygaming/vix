#pragma once
#include <cpp.h>
#include <debug.h>
#include <memory_alloc/memalloc.h>
namespace std {
    template <class T> class vector {
    public:
        vector() {
            uint64_t test1;
            uint64_t *ptr = &test1;
            for (int i = 0; i < 100; i++) {
                testvalue += *ptr;
                ptr--;
            }
            DEBUG_PRINTF("vector CONSTRUCT -> 0x%p\n", testvalue);

            _capacity = 1;
            _size = 0;
            _pointer = (T *)memalloc::single::kmalloc(_capacity * sizeof(T));
        }

        vector(const vector &obj) {
            uint64_t test1;
            uint64_t *ptr = &test1;
            for (int i = 0; i < 100; i++) {
                testvalue += *ptr;
                ptr--;
            }
            DEBUG_PRINTF("vector COPY -> 0x%p\n", testvalue);

            _capacity = 1;
            _size = 0;
            _pointer = (T *)memalloc::single::kmalloc(_capacity * sizeof(T));

            assureSize(obj._size);

            for (int i = 0; i < obj._size; i++) {
                new (&_pointer[i]) T(obj._pointer[i]); // placement new + call copy constructor
            }
        }

        ~vector() {
            DEBUG_PRINTF("vector DESTRUCT -> 0x%p\n", testvalue);
            memalloc::single::kfree(_pointer);
        }

        T &operator[](int i) {
            assertm(i < _capacity, "out of bounds vector access");
            return _pointer[i];
        }

        int capacity() {
            return _capacity;
        }

        int size() {
            return _size;
        }

        void resize(int n) {
            _size = n;
            reallocate(n);
        }

        void reserve(int n) {
            if (n > _capacity) {
                reallocate(n);
            }
        }

        void shrink_to_fit() {
            reallocate(_size);
        }

        void push_back(const T &val) {
            assureSize(_size + 1);
            new (&_pointer[_size - 1]) T(val); // placement new + call copy constructor
        }

        void pop_back() {
            if (_size > 0) {
                _pointer[_size - 1].~T();
                _size--;
            }
        }

        void clear() {
            for (int i = 0; i < _size; i++) {
                _pointer[i].~T();
            }
            _size = 0;
            reallocate(1);
        }

    private:
        T *_pointer = 0;
        int _capacity = 0;
        int _size = 0;

        void assureSize(int n) {
            _size = n;
            if (_size > _capacity) {
                reallocate(_size);
            }
        }

        void reallocate(int capacity) {
            _capacity = capacity;
            if (_capacity <= 0) {
                _capacity = 1;
            }
            _pointer = (T *)memalloc::single::krealloc(_pointer, _capacity * sizeof(T));
        }

        uint32_t testvalue = 0;
    };
}
