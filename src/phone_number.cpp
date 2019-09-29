#include <cassert>
#include <new>
#include <stdexcept>

#include "phonenumbers/phonenumberutil.h"

#include "mask.h"
#include "phone_number.h"
#include "phone_number_constants.h"

namespace {
constexpr size_t country_code_bits_offset = 0;
constexpr size_t even_size_bit_offset = country_code_bits_offset + country_code_bits;
constexpr size_t extension_bits_offset = even_size_bit_offset + 1;
constexpr size_t extension_bits = 2;

constexpr size_t required_bytes(uint32_t value) {
    static_assert(CHAR_BIT == 8, "not implemented for non-8-bit bytes");
    if (value <= std::numeric_limits<uint8_t>::max())
        return sizeof(uint8_t);
    if (value <= std::numeric_limits<uint16_t>::max())
        return sizeof(uint16_t);
    return sizeof(uint32_t);
}

using PhoneNumberUtil = i18n::phonenumbers::PhoneNumberUtil;
static const PhoneNumberUtil* const phoneUtil = PhoneNumberUtil::GetInstance();

constexpr bool is_odd(size_t n) { return (n & 1) == 1; }

Digit to_digit(char digit) {
    if (digit >= '0' && digit <= '9') {
        return static_cast<Digit>(digit - '0');
    }
    // TODO: handle special characters.
    throw std::logic_error("Invalid digit");
}
} // namespace

PhoneNumber* PhoneNumber::make(uint32_t digits, uint32_t extension_digits) {
    const size_t extension_size_bytes = required_bytes(extension_digits);
    const size_t total_digits = extension_size_bytes + digits + extension_digits;
    const bool odd_size = is_odd(total_digits);
    size_t bytes = total_digits / 2;
    if (odd_size) {
        bytes += 1;
    }
    const size_t total_size = offsetof(PhoneNumber, _data) + bytes;

    auto buf = palloc0(total_size);
    auto str = new (buf) PhoneNumber();
    if (str == nullptr) {
        throw std::bad_alloc();
    }

    (void)str->_size; // Avoid unused field warning
    SET_VARSIZE(str, total_size);
    str->set_odd_size(odd_size);
    str->set_extension_size_bytes(extension_size_bytes);
    str->set_extension_size(extension_digits);

    return str;
}

PhoneNumber* PhoneNumber::make(const i18n::phonenumbers::PhoneNumber& number) {
    std::string number_text;
    phoneUtil->GetNationalSignificantNumber(number, &number_text);
    const size_t extension_size = number.has_extension() ? number.extension().size() : 0;
    PhoneNumber* new_number = make(number_text.size(), extension_size);
    new_number->set_country_code(number.country_code());

    for (size_t i = 0; i < number_text.size(); ++i) {
        new_number->set_digit(i, to_digit(number_text[i]));
    }

    // TODO: extension

    return new_number;
}

int PhoneNumber::compare(const PhoneNumber& a, const PhoneNumber& b) noexcept {
    const auto code_a = a.country_code();
    const auto code_b = b.country_code();
    if (code_a < code_b)
        return -1;
    else if (code_b < code_a)
        return 1;
    const size_t size_a = a.size();
    const size_t size_b = b.size();
    for (size_t i = 0; i < std::min(size_a, size_b); ++i) {
        Digit digit_a = a.digit(i);
        Digit digit_b = b.digit(i);
        if (digit_a < digit_b)
            return -1;
        else if (digit_b < digit_a)
            return 1;
    }
    if (size_a < size_b)
        return -1;
    else if (size_b < size_a)
        return 1;
    else
        return 0;
}

PhoneNumber::operator i18n::phonenumbers::PhoneNumber() const {
    i18n::phonenumbers::PhoneNumber number;
    number.set_country_code(country_code());

    uint64_t national_number = 0;
    size_t leading_zeroes = 0;

    for (size_t i = 0; i < size(); ++i) {
        Digit d = digit(i);
        if (d == Digit::N0 && national_number == 0) {
            leading_zeroes += 1;
        } else {
            if (d <= Digit::N9) {
                national_number *= 10;
                national_number += static_cast<uint64_t>(d);
            }
            // TODO: handle extensions, special characters, etc.
        }
    }

    number.set_national_number(national_number);
    number.set_italian_leading_zero(leading_zeroes > 0); // TODO: is this what we want to do?
    number.set_number_of_leading_zeros(leading_zeroes);

    return number;
}

uint16_t PhoneNumber::country_code() const noexcept {
    static_assert(country_code_bits_offset + country_code_bits <= sizeof(_bits) * CHAR_BIT,
        "must have enough bits for country code");
    return get_masked(_bits, country_code_bits, country_code_bits_offset);
}

void PhoneNumber::set_country_code(uint16_t country_code) noexcept {
    _bits = set_masked(_bits, country_code, country_code_bits, country_code_bits_offset);
}

uint32_t PhoneNumber::data_size() const noexcept {
    const uint32_t bytes = VARSIZE(this) - offsetof(PhoneNumber, _data);
    uint32_t count = bytes * 2;
    if (has_odd_size()) {
        count -= 1;
    }
    return count;
}

// Returns the number of bytes used to store the extension size.
uint32_t PhoneNumber::extension_size_bytes() const noexcept {
    static_assert(
        extension_bits_offset + extension_bits <= sizeof(_bits) * CHAR_BIT, "must have enough bits for extension size");
    auto ext_size_flag = get_masked(_bits, extension_bits, extension_bits_offset);
    switch (ext_size_flag) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 4;
    }
    static_assert(extension_bits == 2, "Only 2-bit values are handled.");
    assert(false);
}

void PhoneNumber::set_extension_size_bytes(uint32_t bytes) {
    decltype(_bits) flags = 0;
    switch (bytes) {
    case 0: flags = 0; break;
    case 1: flags = 1; break;
    case 2: flags = 2; break;
    case 4: flags = 3; break;
    default: throw std::logic_error("Invalid number of extension size bytes");
    }
    _bits = set_masked(_bits, flags, extension_bits, extension_bits_offset);
}

uint32_t PhoneNumber::extension_size() const noexcept {
    uint32_t size = 0;
    memcpy(&size, _data, extension_size_bytes());
    return size;
}

void PhoneNumber::set_extension_size(uint32_t size) {
    const size_t bytes = extension_size_bytes();
    if (required_bytes(size) > bytes)
        throw std::logic_error("Not enough space to store extension size");
    memcpy(_data, &size, bytes);
}

Digit PhoneNumber::digit(uint32_t index) const noexcept {
    // TODO: better constants and static_assertions for packing
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    return static_cast<Digit>(get_masked(packed_digits()[byte], 4, odd_index ? 4 : 0));
}

void PhoneNumber::set_digit(uint32_t index, Digit digit) noexcept {
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    auto digits = packed_digits();
    digits[byte] = set_masked(digits[byte], static_cast<uint8_t>(digit), 4, odd_index ? 4 : 0);
}

bool PhoneNumber::has_odd_size() const noexcept {
    static_assert(even_size_bit_offset < sizeof(_bits) * CHAR_BIT, "must have enough bits for odd size flag");
    return get_masked(_bits, 1, even_size_bit_offset) == 1;
}

void PhoneNumber::set_odd_size(bool even) noexcept {
    _bits = set_masked(_bits, static_cast<uint16_t>(even), 1, country_code_bits);
}

bool operator==(const PhoneNumber& a, const PhoneNumber& b) noexcept {
    if (a.size() != b.size())
        return false;
    if (a.country_code() != b.country_code())
        return false;
    // TODO: optimize.
    for (size_t i = 0; i < a.size(); ++i) {
        if (a.digit(i) != b.digit(i))
            return false;
    }
    return true;
}