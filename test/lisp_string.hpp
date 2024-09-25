/*
 * 这个好像没用了，但是花了精力不就删掉了😭
*/

#pragma once

#include <cstddef>
#include <cstring>

namespace austlisp {

struct String {
    String(const char* str) noexcept {
        _construct_from_c_str(str);
    }
    String(const String& other) {
        size_t other_size = other.size();
        _str_begin        = new char[other_size];
        _str_end          = _str_begin + other_size;
        memcpy(_str_begin, other._str_begin, sizeof(char) * other_size);
    }
    String(String&& other) noexcept {
        _str_begin       = other._str_begin;
        _str_end         = other._str_end;
        other._str_begin = other._str_end = nullptr;
    }
    const String& operator=(const char* str) noexcept {
        _construct_from_c_str(str);
        return *this;
    }
    const String& operator=(const String& other) {
        if (this != &other) {
            size_t other_size = other.size();
            _str_begin        = new char[other_size];
            _str_end          = _str_begin + other_size;
            memcpy(_str_begin, other._str_begin, sizeof(char) * other_size);
        }
        return *this;
    }
    const String& operator=(String&& other) noexcept {
        if (this != &other) {
            _str_begin       = other._str_begin;
            _str_end         = other._str_end;
            other._str_begin = other._str_end = nullptr;
        }
        return *this;
    }
    ~String() {
        delete[] _str_begin;
        _str_end = nullptr;
    }

    const char operator[](const size_t index) noexcept {
        return _str_begin[index];
    }

    constexpr size_t size() const noexcept {
        return static_cast<size_t>(_str_end - _str_begin);
    }

private:
    void _construct_from_c_str(const char* str) {
        size_t size = std::strlen(str);
        _str_begin  = new char[size];
        for (int i = 0; i < size; ++i) {
            _str_begin[i] = str[i];
        }
        _str_end = _str_begin + size;
    }

private:
    char* _str_begin;
    char* _str_end;
};

} // namespace austlisp
