#pragma once

#include <cstdint>
#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

#include "mask.h"

/**
 * Raised when a phone number is too long to fit in a PackedPhoneNumber
 * 
 * This shouldn't happen for any phone numbers that meet ITU specifications.
 */
class PhoneNumberTooLongException : public std::runtime_error {
    public:
    PhoneNumberTooLongException(const i18n::phonenumbers::PhoneNumber& number, const char* msg);

    /// Returns the original phone number object
    const i18n::phonenumbers::PhoneNumber& number() const {
        return _number;
    }

    //TODO: just get the number string from which the PhoneNumber was parsed? (if it exists...)
    std::string number_string() const;
    private:
    i18n::phonenumbers::PhoneNumber _number;
};

/**
 * Stores a phone number (packed into a 64-bit integer)
 */
class PackedPhoneNumber {
    public:
    /// The largest possible country code
    static constexpr size_t max_country_code = 999;
    /// The maximum number of leading zeroes in a national phone number
    static constexpr size_t max_leading_zeros = 15;
    /// The largest possible national number
    static constexpr size_t max_national_number = 999999999999999;

    /// The number of bits reserved for a country code
    static constexpr size_t country_code_bits = 10;
    /// The number of bits reserved for the leading zero count
    static constexpr size_t leading_zeros_bits = 4;
    /// The number of bits reserved for the national number
    static constexpr size_t national_number_bits = 50;

    // Bit offsets of the number components
    static constexpr size_t country_code_offset = 0;
    static constexpr size_t leading_zeros_offset = country_code_offset + country_code_bits;
    static constexpr size_t national_number_offset = leading_zeros_offset + leading_zeros_bits;

    PackedPhoneNumber(const i18n::phonenumbers::PhoneNumber& number);

    bool operator==(const PackedPhoneNumber other) const {
        return this->_data == other._data;
    }

    bool operator != (const PackedPhoneNumber other) const {
        return !(*this == other);
    }

    /// Casts this object to a libphonenumber PhoneNumber object
    operator i18n::phonenumbers::PhoneNumber() const;

    /*
     * Compares to another PackedPhoneNumber using a fast collation heuristic
     *
     * May not produce intuitive results for numbers with the same
     * country code but different lengths
     *
     * Returns:
     * - <0 (if a < b)
     * - 0 (if a == b)
     * - >0 (if a > b)
     */
    int64_t compare_fast(PackedPhoneNumber other) const {
        return other._data - this->_data;
    }

    /// Returns the number's country code
    google::protobuf::uint32 country_code() const {
        return get_masked(_data, country_code_bits, country_code_offset);
    }

    /// Sets the number's country code
    void country_code(uint32_t value) {
        _data = set_masked(_data, static_cast<uint64_t>(value), country_code_bits, country_code_offset);
    }

    /// Returns the national number
    uint64_t national_number() const {
        return get_masked(_data, national_number_bits, national_number_offset);
    }

    /// Sets the national number
    void national_number(uint64_t value) {
        _data = set_masked(_data, value, national_number_bits, national_number_offset);
    }

    /// Returns the number of leading zeros
    uint64_t leading_zeros() const {
        return get_masked(_data, leading_zeros_bits, leading_zeros_offset);
    }

    /// Sets the number of leading zeros
    void leading_zeros(uint64_t value) {
        _data = set_masked(_data, value, leading_zeros_bits, leading_zeros_offset);
    }

    private:
    uint64_t _data;
};

/*
 * If the size of the PackedPhoneNumber class changes for any reason, it will trip this assertion and remind us to
 * update the SQL definition of the short_phone_number type.
 */
static_assert(sizeof(PackedPhoneNumber) == 8, "unexpected size for PackedPhoneNumber");
