#pragma once
#include <../config.h>
#include <types.h>

#if (CONFIG_ENDIAN_BIG == 0)
inline uint16_t endian_assure_little(uint16_t value) {
    return value;
}

inline uint32_t endian_assure_little(uint32_t value) {
    return value;
}

inline uint64_t endian_assure_little(uint64_t value) {
    return value;
}

inline uint16_t endian_assure_big(uint16_t value) {
    return (value >> 8) | ((value & 0xFF) << 8);
}

inline uint32_t endian_assure_big(uint32_t value) {
    return (value >> 24) | ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8) | (value << 24);
}

inline uint64_t endian_assure_big(uint64_t value) {
    return (value >> 56) | ((value & 0x00FF000000000000) >> (5 * 8)) | ((value & 0x0000FF0000000000) >> (3 * 8)) | ((value & 0x000000FF00000000) >> 8) | ((value & 0x00000000FF000000) << 8) |
           ((value & 0x0000000000FF0000) << (3 * 8)) | ((value & 0x000000000000FF00) << (5 * 8)) | (value << 56);
}
#else
inline uint16_t endian_assure_big(uint16_t value) {
    return value;
}

inline uint32_t endian_assure_big(uint32_t value) {
    return value;
}

inline uint64_t endian_assure_big(uint64_t value) {
    return value;
}

inline uint16_t endian_assure_little(uint16_t value) {
    return (value >> 8) | ((value & 0xFF) << 8);
}

inline uint32_t endian_assure_little(uint32_t value) {
    return (value >> 24) | ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8) | (value << 24);
}

inline uint64_t endian_assure_little(uint64_t value) {
    return (value >> 56) | ((value & 0x00FF000000000000) >> (5 * 8)) | ((value & 0x0000FF0000000000) >> (3 * 8)) | ((value & 0x000000FF00000000) >> 8) | ((value & 0x00000000FF000000) << 8) |
           ((value & 0x0000000000FF0000) << (3 * 8)) | ((value & 0x000000000000FF00) << (5 * 8)) | (value << 56);
}
#endif
