#pragma once

extern "C" {
#include "postgres.h"
}

#include <cstdint>

namespace i18n {
namespace phonenumbers {
    class PhoneNumber;
}
} // namespace i18n

enum class Digit : uint8_t {
    N0,
    N1,
    N2,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,
    Star,
    Pound,
    // TODO: extension separator? (may be implementation detail of PhoneNumber
    // or something...)
};

class PhoneNumber {
  public:
    static PhoneNumber* make(uint32_t size);
    static PhoneNumber* make(const i18n::phonenumbers::PhoneNumber& number);

    static int compare(const PhoneNumber& a, const PhoneNumber& b) noexcept;

    operator i18n::phonenumbers::PhoneNumber() const;

    uint16_t country_code() const noexcept;
    void set_country_code(uint16_t country_code) noexcept;
    size_t size() const noexcept;
    Digit get(size_t index) const noexcept;
    void set(size_t index, Digit digit) noexcept;

  private:
    uint32_t _size = 0;
    uint16_t _bits = 0;
    uint8_t _digits[FLEXIBLE_ARRAY_MEMBER];

    PhoneNumber() = default;

    bool has_odd_size() const noexcept;
    void set_odd_size(bool odd) noexcept;
};

bool operator==(const PhoneNumber& a, const PhoneNumber& b) noexcept;
inline bool operator!=(const PhoneNumber& a, const PhoneNumber& b) noexcept { return !operator==(a, b); }
inline bool operator<(const PhoneNumber& a, const PhoneNumber& b) noexcept { return PhoneNumber::compare(a, b) < 0; }
inline bool operator<=(const PhoneNumber& a, const PhoneNumber& b) noexcept { return PhoneNumber::compare(a, b) <= 0; }
inline bool operator>(const PhoneNumber& a, const PhoneNumber& b) noexcept { return PhoneNumber::compare(a, b) > 0; }
inline bool operator>=(const PhoneNumber& a, const PhoneNumber& b) noexcept { return PhoneNumber::compare(a, b) >= 0; }
