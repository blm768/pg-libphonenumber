#pragma once

extern "C" {
    #include "postgres.h"
}

#include <cstdint>

namespace i18n {
    namespace phonenumbers {
        class PhoneNumber;
    }
}

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
    // TODO: extension separator? (may be implementation detail of PhoneNumber or something...)
};

class PhoneNumber {
public:
    static PhoneNumber* make(uint32_t size);
    static PhoneNumber* make(const i18n::phonenumbers::PhoneNumber& number);

    operator i18n::phonenumbers::PhoneNumber() const;

    uint16_t country_code() const;
    size_t size() const;
    Digit get(size_t index) const;
    void set(size_t index, Digit digit);

private:
    uint32_t _size = 0;
    uint16_t _bits = 0;
    static constexpr size_t extra_size = VARHDRSZ + sizeof(_bits); // NOTE: must stay in sync with size of data members above.
    uint8_t _digits[FLEXIBLE_ARRAY_MEMBER];

    PhoneNumber() = default;

    bool has_odd_size() const;
    void set_odd_size(bool odd);
};