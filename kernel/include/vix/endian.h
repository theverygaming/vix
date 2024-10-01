#pragma once
#include <vix/config.h>
#include <vix/types.h>

// clang-format off
#define E_SWAP_16(x) ((x >> 8) | ((x & 0xFF) << 8))
#define E_SWAP_32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))
#define E_SWAP_64(x) ((x >> 56) | ((x & 0x00FF000000000000) >> (5 * 8)) | ((x & 0x0000FF0000000000) >> (3 * 8)) | ((x & 0x000000FF00000000) >> 8) | ((x & 0x00000000FF000000) << 8) |((x & 0x0000000000FF0000) << (3 * 8)) | ((x & 0x000000000000FF00) << (5 * 8)) | (x << 56))
// clang-format on

#if (CONFIG_ENDIAN_BIG == 0)
#define LE_16(x) (x)
#define LE_32(x) (x)
#define LE_64(x) (x)
#define BE_16(x) E_SWAP_16(x)
#define BE_32(x) E_SWAP_32(x)
#define BE_64(x) E_SWAP_64(x)
#else
#define LE_16(x) E_SWAP_16(x)
#define LE_32(x) E_SWAP_32(x)
#define LE_64(x) E_SWAP_64(x)
#define BE_16(x) (x)
#define BE_32(x) (x)
#define BE_64(x) (x)
#endif

#ifdef __cplusplus
namespace endian {
    constexpr uint16_t c_be_16(uint16_t x) {
        return BE_16(x);
    }

    constexpr uint32_t c_be_32(uint32_t x) {
        return BE_32(x);
    }

    constexpr uint64_t c_be_64(uint64_t x) {
        return BE_64(x);
    }

    constexpr uint16_t c_le_16(uint16_t x) {
        return LE_16(x);
    }

    constexpr uint32_t c_le_32(uint32_t x) {
        return LE_32(x);
    }

    constexpr uint64_t c_le_64(uint64_t x) {
        return LE_64(x);
    }
}
#endif
