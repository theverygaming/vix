#pragma once
#include <cpp.h>
#include <debug.h>
#include <mm/kmalloc.h>
#include <stdlib.h>

namespace std {
    /* this is not threadsafe! */
    template <class T>
    class vector {
    public:
        vector() {
            _capacity = 1;
            _size = 0;
            if (alignof(T) > 1 && false) {
                _pointer = (T *)mm::kmalloc_aligned(_capacity * sizeof(T), alignof(T));
            } else {
                _pointer = (T *)mm::kmalloc(_capacity * sizeof(T));
            }
        }

        vector(const vector &obj) {
            _capacity = 1;
            _size = 0;
            if (alignof(T) > 1 && false) {
                _pointer = (T *)mm::kmalloc_aligned(_capacity * sizeof(T), alignof(T));
            } else {
                _pointer = (T *)mm::kmalloc(_capacity * sizeof(T));
            }

            assureSize(obj._size);

            for (size_t i = 0; i < obj._size; i++) {
                new (&_pointer[i]) T(obj._pointer[i]); // placement new + call copy constructor
            }
        }

        ~vector() {
            for (size_t i = 0; i < _size; i++) {
                _pointer[i].~T();
            }
            if (alignof(T) > 1 && false) {
                // mm::kfree_aligned(_pointer);
            } else {
                mm::kfree(_pointer);
            }
        }

        T &operator[](size_t i) {
            assertm(i < _size, "out of bounds vector access");
            return _pointer[i];
        }

        T &operator[](size_t i) const {
            assertm(i < _size, "out of bounds vector access");
            return _pointer[i];
        }

        vector<T> &operator=(const vector<T> &obj) {
            _capacity = 1;
            _size = 0;
            if (alignof(T) > 1 && false) {
                _pointer = (T *)mm::kmalloc_aligned(_capacity * sizeof(T), alignof(T));
            } else {
                _pointer = (T *)mm::kmalloc(_capacity * sizeof(T));
            }

            assureSize(obj._size);

            for (size_t i = 0; i < obj._size; i++) {
                new (&_pointer[i]) T(obj._pointer[i]); // placement new + call copy constructor
            }
            return *this;
        }

        size_t capacity() {
            return _capacity;
        }

        size_t size() {
            return _size;
        }

        void resize(size_t n) {
            _size = n;
            reallocate(n);
        }

        void reserve(size_t n) {
            if (n > _capacity) {
                reallocate(n);
            }
        }

        void shrink_to_fit() {
            reallocate(_size);
        }

        void push_back(const T &val) {
            assureSize(_size + 1, 2);
            new (&_pointer[_size - 1]) T(val); // placement new + call copy constructor
        }

        void pop_back() {
            if (_size > 0) {
                _size--;
                _pointer[_size].~T();
            }
        }

        void erase(size_t first, size_t last) { // TODO: have this not call the other erase function so it's faster
            if (first > last) {
                return;
            }
            size_t count = (last - first) + 1;
            for (size_t i = 0; i < count; i++) {
                erase(first);
            }
        }

        void erase(size_t index) {
            if (index < _size) {
                _pointer[index].~T();
                if ((index + 1) < _size) {
                    memmove(&_pointer[index], &_pointer[index + 1], (_size - (index + 1)) * sizeof(T));
                }
                _size--;
            }
        }

        void clear() {
            for (size_t i = 0; i < _size; i++) {
                _pointer[i].~T();
            }
            _size = 0;
            reallocate(1);
        }

    private:
        T *_pointer = nullptr;
        size_t _capacity = 0;
        size_t _size = 0;

        void assureSize(size_t n, size_t preallocExtra = 0) {
            _size = n;
            if (_size > _capacity) {
                reallocate(_size + preallocExtra);
            }
        }

        void reallocate(size_t capacity) {
            _capacity = capacity;
            if (_capacity == 0) {
                _capacity = 1;
            }

            if (alignof(T) > 1 && false) {
                // TODO: aligned krealloc + aligned free
                T *newptr = (T *)mm::kmalloc_aligned(_capacity * sizeof(T), alignof(T));
                memcpy(newptr, _pointer, _capacity * sizeof(T));
                // mm::kfree_aligned(_pointer);
                _pointer = newptr;
            } else {
                _pointer = (T *)mm::krealloc(_pointer, _capacity * sizeof(T));
            }
        }
    };
}
