#pragma once

namespace std {
    template <class T> const T &max(const T &a, const T &b) {
        return (a < b) ? b : a;
    }

    template <class T> const T &min(const T &a, const T &b) {
        return (b < a) ? b : a;
    }

    template <class T> constexpr const T clamp(const T &v, const T &lo, const T &hi) {
        T val = v;
        val = val < lo ? lo : val;
        val = val > hi ? hi : val;
        return val;
    }
}
