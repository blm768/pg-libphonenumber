#include <algorithm>

template<typename T> constexpr T mask(size_t bits, size_t offset = 0) {
    return (((T)1 << bits) - 1) << offset;
}

template<typename T> constexpr T get_masked(T data, size_t bits, size_t offset) {
    return (data >> offset) & mask<T>(bits);
}

//TODO: support typeof(data) != typeof(value)?
template<typename T> constexpr T set_masked(T data, T value, size_t bits, size_t offset) {
    return (data & ~mask<T>(bits, offset)) | ((value & mask<T>(bits)) << offset);
}
