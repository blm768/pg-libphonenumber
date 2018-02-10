#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

#include "mask.h"

/**
 * Raised when a phone number is too long to fit in a ShortPhoneNumber
 * 
 * This shouldn't happen for any valid phone numbers.
 */
class PhoneNumberTooLongException : public std::runtime_error {
    public:
    PhoneNumberTooLongException(const i18n::phonenumbers::PhoneNumber& number, const char* msg);

    /// Returns the original phone number object
    i18n::phonenumbers::PhoneNumber number() const {
        return _number;
    }

    //TODO: just get the number string from which the PhoneNumber was parsed? (if it exists...)
    std::string number_string() const;
    private:
    i18n::phonenumbers::PhoneNumber _number;

    static const i18n::phonenumbers::PhoneNumberUtil* const phoneUtil;
};

/**
 * Stores a phone number (packed into a 64-bit integer)
 */
class ShortPhoneNumber {
    public:
    enum : size_t {
        /// The largest possible country code
        MAX_COUNTRY_CODE = 999,
        /// The maximum number of leading zeroes in a national phone number
        MAX_LEADING_ZEROS = 15,
        /// The largest possible national number
        MAX_NATIONAL_NUMBER = 999999999999999,
    };

    enum : size_t {
        /// The number of bits reserved for a country code
        COUNTRY_CODE_BITS = 10,
        /// The number of bits reserved for the leading zero count
        LEADING_ZEROS_BITS = 4,
        /// The number of bits reserved for the national number
        NATIONAL_NUMBER_BITS = 50,
    };

    // Bit offsets of the number components
    enum : size_t {
        COUNTRY_CODE_OFFSET = 0,
        LEADING_ZEROS_OFFSET = COUNTRY_CODE_OFFSET + COUNTRY_CODE_BITS,
        NATIONAL_NUMBER_OFFSET = LEADING_ZEROS_OFFSET + LEADING_ZEROS_BITS,
    };

    ShortPhoneNumber(i18n::phonenumbers::PhoneNumber number);

    bool operator == (const ShortPhoneNumber other) const {
        return this->_data == other._data;
    }

    bool operator != (const ShortPhoneNumber other) const {
        return !(*this == other);
    }

    /// Casts this object to a libphonenumber PhoneNumber object
    operator i18n::phonenumbers::PhoneNumber() const;

    /*
     * Compares to another ShortPhoneNumber using a fast collation heuristic
     *
     * May not produce intuitive results for numbers with the same
     * country code but different lengths
     *
     * Returns:
     * - <0 (if a < b)
     * - 0 (if a == b)
     * - >0 (if a > b)
     */
    google::protobuf::int64 compare_fast(ShortPhoneNumber other) const {
        return other._data - this->_data;
    }

    /// Returns the number's country code
    google::protobuf::uint32 country_code() const {
        return get_masked(_data, COUNTRY_CODE_BITS, COUNTRY_CODE_OFFSET);
    }

    /// Sets the number's country code
    void country_code(google::protobuf::uint32 value) {
        _data = set_masked(_data, (google::protobuf::uint64)value, COUNTRY_CODE_BITS, COUNTRY_CODE_OFFSET);
    }

    /// Returns the national number
    google::protobuf::uint64 national_number() const {
        return get_masked(_data, NATIONAL_NUMBER_BITS, NATIONAL_NUMBER_OFFSET);
    }

    /// Sets the national number
    void national_number(google::protobuf::uint64 value) {
        _data = set_masked(_data, value, NATIONAL_NUMBER_BITS, NATIONAL_NUMBER_OFFSET);
    }

    /// Returns the number of leading zeros
    google::protobuf::uint64 leading_zeros() const {
        return get_masked(_data, LEADING_ZEROS_BITS, LEADING_ZEROS_OFFSET);
    }

    /// Sets the number of leading zeros
    void leading_zeros(google::protobuf::uint64 value) {
        _data = set_masked(_data, value, LEADING_ZEROS_BITS, LEADING_ZEROS_OFFSET);
    }

    private:
    google::protobuf::uint64 _data;
};

/*
 * If the size of the ShortPhoneNumber class changes for any reason, it will trip this assertion and remind us to
 * update the SQL definition of the short_phone_number type.
 */
static_assert(sizeof(ShortPhoneNumber) == 8);
