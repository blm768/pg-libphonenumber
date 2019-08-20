#include <new>
#include <stdexcept>

#include "phonenumbers/phonenumberutil.h"

#include "phone_number.h"
#include "mask.h"
#include "phone_number_constants.h"

namespace {
    using PhoneNumberUtil = i18n::phonenumbers::PhoneNumberUtil;
    static const PhoneNumberUtil* const phoneUtil = PhoneNumberUtil::GetInstance();

    constexpr bool is_odd(size_t n) {
        return (n & 1) == 1;
    }

    Digit to_digit(char digit) {
        if (digit >= '0' && digit <= '9') {
            return static_cast<Digit>(digit - '0');
        }
        // TODO: handle special characters.
        throw std::logic_error("Invalid digit");
    }
}

PhoneNumber* PhoneNumber::make(uint32_t size) {
    const bool odd_size = is_odd(size);
    size_t bytes = size / 2;
    if (odd_size) {
        bytes += 1;
    }
    const size_t total_size = extra_size + bytes;

    auto buf = palloc(total_size);
    auto str = new(buf) PhoneNumber();
    if (str == nullptr) {
        throw std::bad_alloc();
    }

    (void)str->_size; // Avoid unused field warning
    SET_VARSIZE(str, total_size);
    str->set_odd_size(odd_size);
    memset(&str->_digits, 0, bytes);

    return str;
}

PhoneNumber* PhoneNumber::make(const i18n::phonenumbers::PhoneNumber& number) {
    std::string number_text;
    phoneUtil->GetNationalSignificantNumber(number, &number_text);
    const size_t extension_size = number.has_extension() ? number.extension().size() : 0;
    const size_t num_digits = number_text.size() + extension_size;
    PhoneNumber* new_number = make(num_digits);
    new_number->set_country_code(number.country_code());

    for (size_t i = 0; i < number_text.size(); ++i) {
        new_number->set(i, to_digit(number_text[i]));
    }

    // TODO: extension

    return new_number;
}

PhoneNumber::operator i18n::phonenumbers::PhoneNumber() const {
    i18n::phonenumbers::PhoneNumber number;
    number.set_country_code(country_code());

    uint64_t national_number = 0;
    size_t leading_zeroes = 0;

    for (size_t i = 0; i < size(); ++i) {
        Digit digit = get(i);
        if (digit == Digit::N0 && national_number == 0) {
            leading_zeroes += 1;
        } else {
            if (digit <= Digit::N9) {
                national_number *= 10;
                national_number += static_cast<uint64_t>(digit);
            }
            // TODO: handle extensions, special characters, etc.
        }
    }

    number.set_national_number(national_number);
    number.set_italian_leading_zero(leading_zeroes > 0); // TODO: is this what we want to do?
    number.set_number_of_leading_zeros(leading_zeroes);

    return number;
}

uint16_t PhoneNumber::country_code() const {
    return get_masked(_bits, country_code_bits, 0);
}

void PhoneNumber::set_country_code(uint16_t country_code) {
    _bits = set_masked(_bits, country_code, country_code_bits, 0);
}

size_t PhoneNumber::size() const {
    const size_t bytes = VARSIZE(this) - extra_size;
    size_t count = bytes * 2;
    if (has_odd_size()) {
        count -= 1;
    }
    return count;
}

Digit PhoneNumber::get(size_t index) const {
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    return static_cast<Digit>(get_masked(_digits[byte], 4, odd_index ? 4 : 0));
}

void PhoneNumber::set(size_t index, Digit digit) {
    const bool odd_index = is_odd(index);
    const size_t byte = index / 2;
    _digits[byte] = set_masked(_digits[byte], static_cast<uint8_t>(digit), 4, odd_index ? 4 : 0);
}

bool PhoneNumber::has_odd_size() const {
    return get_masked(_bits, 1, country_code_bits) == 1;
}

void PhoneNumber::set_odd_size(bool even) {
    _bits = set_masked(_bits, static_cast<uint16_t>(even), 1, country_code_bits);
}

bool operator==(const PhoneNumber &a, const PhoneNumber &b) noexcept {
    if (a.size() != b.size())
        return false;
    if (a.country_code() != b.country_code())
        return false;
    // TODO: optimize.
    for (size_t i = 0; i < a.size(); ++i) {
        if (a.get(i) != b.get(i))
            return false;
    }
    return true;
}