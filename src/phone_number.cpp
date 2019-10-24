#include <cassert>
#include <new>
#include <stdexcept>

#include "phonenumbers/phonenumberutil.h"

#include "error_handling.h"
#include "mask.h"
#include "phone_number.h"
#include "phone_number_constants.h"

namespace {
constexpr size_t country_code_bits_offset = 0;
constexpr size_t even_size_bit_offset = country_code_bits_offset + country_code_bits;
constexpr size_t extension_size_bits_offset = even_size_bit_offset + 1;
constexpr size_t extension_size_bits = 5;
constexpr size_t max_extension_size = (1 << extension_size_bits) - 1;

const char* extension_too_long_msg = "exceeded maximum extension length";

using PhoneNumberUtil = i18n::phonenumbers::PhoneNumberUtil;
static const PhoneNumberUtil* const phoneUtil = PhoneNumberUtil::GetInstance();

constexpr bool is_odd(size_t n) { return (n & 1) == 1; }

Digit to_digit(char digit) {
    if (digit >= '0' && digit <= '9')
        return static_cast<Digit>(digit - '0');
    if (digit == '*')
        return Digit::Star;
    if (digit == '#')
        return Digit::Pound;
    throw std::logic_error("Invalid digit");
}
} // namespace

PhoneNumber* PhoneNumber::make(uint32_t digits, uint32_t extension_digits) {
    const size_t total_digits = digits + extension_digits;
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
    str->set_extension_size(extension_digits);

    return str;
}

PhoneNumber* PhoneNumber::make(const i18n::phonenumbers::PhoneNumber& number) {
    std::string number_text;
    phoneUtil->GetNationalSignificantNumber(number, &number_text);
    const size_t extension_size = number.has_extension() ? number.extension().size() : 0;
    if (extension_size > max_extension_size)
        throw PhoneNumberTooLongException(number, extension_too_long_msg);
    PhoneNumber* new_number = make(number_text.size(), extension_size);
    new_number->set_country_code(number.country_code());

    for (size_t i = 0; i < number_text.size(); ++i) {
        new_number->set_digit(i, to_digit(number_text[i]));
    }

    if (extension_size > 0) {
        const std::string& extension = number.extension();
        for (size_t i = 0; i < extension.size(); ++i) {
            new_number->set_ext_digit(i, to_digit(extension[i]));
        }
    }

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

uint32_t PhoneNumber::extension_size() const noexcept {
    static_assert(extension_size_bits_offset + extension_size_bits <= sizeof(_bits) * CHAR_BIT,
        "must have enough bits for extension size");
    return get_masked(_bits, extension_size_bits, extension_size_bits_offset);
}

void PhoneNumber::set_extension_size(uint32_t size) {
    if (size > max_extension_size)
        throw std::logic_error(extension_too_long_msg);
    _bits = set_masked(_bits, static_cast<decltype(_bits)>(size), extension_size_bits, extension_size_bits_offset);
}

Digit PhoneNumber::digit(uint32_t index) const noexcept {
    // TODO: better constants and static_assertions for packing
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    return static_cast<Digit>(get_masked(_data[byte], 4, odd_index ? 4 : 0));
}

void PhoneNumber::set_digit(uint32_t index, Digit digit) noexcept {
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    _data[byte] = set_masked(_data[byte], static_cast<uint8_t>(digit), 4, odd_index ? 4 : 0);
}

Digit PhoneNumber::ext_digit(uint32_t index) const noexcept { return digit(index + size()); }

void PhoneNumber::set_ext_digit(uint32_t index, Digit digit) noexcept { set_digit(index + size(), digit); }

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