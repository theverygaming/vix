#pragma once
#include <cpp.h>
#include <debug.h>
#include <memory_alloc/memalloc.h>
#include <stdlib.h>

namespace std {
    class string {
    public:
        string() {
            initString();
        }

        string(const string &obj) {
            initString();

            operator=(obj);
        }

        string(const char *s) {
            initString();

            operator=(s);
        }

        string(const char *s, size_t n) {
            initString();

            assureSize(n);
            memcpy(_pointer, s, n);
        }

        ~string() {
            mm::kfree(_pointer);
        }

        char &operator[](size_t i) {
            assertm(i < _capacity, "out of bounds string access");
            return _pointer[i];
        }

        bool operator==(const string &str) {
            if (str._size != _size) {
                return false;
            }
            for (size_t i = 0; i < _size; i++) {
                if (str._pointer[i] != _pointer[i]) {
                    return false;
                }
            }
            return true;
        }

        string &operator=(const string &str) {
            assureSize(str._size);
            memcpy(_pointer, str._pointer, str._size);
            return *this;
        }

        string &operator=(const char *s) {
            size_t len = strlen(s);
            assureSize(len);
            memcpy(_pointer, s, len);
            return *this;
        }

        string &operator=(char c) {
            assureSize(1);
            _pointer[0] = c;
            return *this;
        }

        string &operator+=(const string &str) {
            append(str);
            return *this;
        }

        string &operator+=(const char *s) {
            append(s);
            return *this;
        }

        string &operator+=(char c) {
            push_back(c);
            return *this;
        }

        string &append(const string &str) {
            assureSize(_size + str._size);
            memcpy(&_pointer[_size - str._size], str._pointer, str._size);
            return *this;
        }

        string &append(const char *s) {
            size_t len = strlen(s);
            assureSize(_size + len);
            memcpy(&_pointer[_size - len], s, len);
            return *this;
        }

        string &append(const char *s, size_t n) {
            assureSize(_size + n);
            memcpy(&_pointer[_size - n], s, n);
            return *this;
        }

        const char *c_str() {
            if (_capacity <= _size) {
                reallocate(_size + 1);
            }
            _pointer[_size] = '\0';
            return _pointer;
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

        void push_back(char c) {
            assureSize(_size + 1);
            _pointer[_size - 1] = c;
        }

        void pop_back() {
            if (_size > 0) {
                _size--;
            }
        }

        void clear() {
            _size = 0;
            reallocate(1);
        }

    private:
        char *_pointer = 0;
        size_t _capacity = 0;
        size_t _size = 0;

        void assureSize(size_t n) {
            _size = n;
            if (_size > _capacity) {
                reallocate(_size);
            }
        }

        void reallocate(size_t capacity) {
            _capacity = capacity;
            if (_capacity <= 0) {
                _capacity = 1;
            }
            _pointer = (char *)mm::krealloc(_pointer, _capacity * sizeof(char));
        }

        void initString() {
            _capacity = 1;
            _size = 0;
            _pointer = (char *)mm::kmalloc(_capacity * sizeof(char));
        }
    };
}

std::string operator+(const std::string &lhs, const std::string &rhs);

std::string operator+(const std::string &lhs, const char *rhs);

std::string operator+(const char *lhs, const std::string &rhs);

std::string operator+(const std::string &lhs, char rhs);

std::string operator+(char lhs, const std::string &rhs);
