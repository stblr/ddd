#pragma once

#include <common/Types.hh>

class Bytes {
public:
    template <typename T>
    static T ReadBE(const u8 *src, size_t offset) {
#ifdef __CWCC__
        return *reinterpret_cast<const T *>(src + offset);
#else
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * (sizeof(T) - i - 1));
        }
        return val;
#endif
    }

    template <typename T>
    static T ReadLE(const u8 *src, size_t offset) {
        T val = 0;
#ifdef __CWCC__
        T v = ReadBE<T>(src, offset);
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= (v >> (8 * i) & 0xff) << (8 * (sizeof(T) - i - 1));
        }
#else
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * i);
        }
#endif
        return val;
    }

    template <typename T>
    static void WriteBE(u8 *dst, size_t offset, T val) {
#ifdef __CWCC__
        *reinterpret_cast<T *>(dst + offset) = val;
#else
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * (sizeof(T) - i - 1));
        }
#endif
    }

    template <typename T>
    static void WriteLE(u8 *dst, size_t offset, T val) {
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * i);
        }
    }

private:
    Bytes();
};

#ifdef __CWCC__
template <>
inline s16 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lhbrx(src, offset);
}

template <>
inline s32 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lwbrx(src, offset);
}

template <>
inline s64 Bytes::ReadLE(register const u8 *src, size_t offset) {
    s64 val = 0;
    val |= static_cast<s64>(__lwbrx(src, offset + 0)) << 0;
    val |= static_cast<s64>(__lwbrx(src, offset + 4)) << 32;
    return val;
}

template <>
inline u16 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lhbrx(src, offset);
}

template <>
inline u32 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lwbrx(src, offset);
}

template <>
inline u64 Bytes::ReadLE(register const u8 *src, size_t offset) {
    u64 val = 0;
    val |= static_cast<u64>(__lwbrx(src, offset + 0)) << 0;
    val |= static_cast<u64>(__lwbrx(src, offset + 4)) << 32;
    return val;
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s16 val) {
    __sthbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s32 val) {
    __stwbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s64 val) {
    __stwbrx(val >> 32, dst, offset + 4);
    __stwbrx(val >> 0, dst, offset + 0);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u16 val) {
    __sthbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u32 val) {
    __stwbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u64 val) {
    __stwbrx(val >> 32, dst, offset + 4);
    __stwbrx(val >> 0, dst, offset + 0);
}
#endif
