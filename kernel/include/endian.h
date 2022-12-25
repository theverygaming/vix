#pragma once
#include <config.h>
#include <types.h>

// clang-format off
#if (CONFIG_ENDIAN_BIG == 0)
#define LE_16(x) ()
#define LE_32(x) ()
#define LE_64(x) ()
#define BE_16(x) ((x >> 8) | ((x & 0xFF) << 8))
#define BE_32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))
#define BE_64(x) ((x >> 56) | ((x & 0x00FF000000000000) >> (5 * 8)) | ((x & 0x0000FF0000000000) >> (3 * 8)) | ((x & 0x000000FF00000000) >> 8) | ((x & 0x00000000FF000000) << 8) |((x & 0x0000000000FF0000) << (3 * 8)) | ((x & 0x000000000000FF00) << (5 * 8)) | (x << 56))
#else
#define LE_16(x) ((x >> 8) | ((x & 0xFF) << 8))
#define LE_32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))
#define LE_64(x) ((x >> 56) | ((x & 0x00FF000000000000) >> (5 * 8)) | ((x & 0x0000FF0000000000) >> (3 * 8)) | ((x & 0x000000FF00000000) >> 8) | ((x & 0x00000000FF000000) << 8) |((x & 0x0000000000FF0000) << (3 * 8)) | ((x & 0x000000000000FF00) << (5 * 8)) | (x << 56))
#define BE_16(x) ()
#define BE_32(x) ()
#define BE_64(x) ()
#endif
// clang-format on
