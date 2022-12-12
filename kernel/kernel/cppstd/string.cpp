#include <cppstd/string.h>

std::string operator+(const std::string &lhs, const std::string &rhs) {
    std::string ret = lhs;
    ret += rhs;
    return ret;
}

std::string operator+(const std::string &lhs, const char *rhs) {
    std::string ret = lhs;
    ret += rhs;
    return ret;
}

std::string operator+(const char *lhs, const std::string &rhs) {
    std::string ret = lhs;
    ret += rhs;
    return ret;
}

std::string operator+(const std::string &lhs, char rhs) {
    std::string ret = lhs;
    ret += rhs;
    return ret;
}

std::string operator+(char lhs, const std::string &rhs) {
    std::string ret;
    ret.push_back(lhs);
    ret += rhs;
    return ret;
}
