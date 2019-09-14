#pragma once

#include <cstddef>
#include <type_traits>

// Returns an instance of T with the given number of bits set to 1, starting at the given offset
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type mask(size_t num_bits, size_t offset = 0) {
    return (((T)1 << num_bits) - 1) << offset;
}

// Returns the given number of bits from the data parameter, starting at the given offset
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type get_masked(
    T data, size_t num_bits, size_t offset) {
    return (data >> offset) & mask<T>(num_bits);
}

/**
 * Returns a copy of the given data with the given number of bits (starting at the given offset)
 * set to match the given binary value
 */
// TODO: support typeof(data) != typeof(value)?
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type set_masked(
    T data, T value, size_t num_bits, size_t offset) {
    return (data & ~mask<T>(num_bits, offset)) | ((value & mask<T>(num_bits)) << offset);
}
