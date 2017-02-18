#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

#include "mask.h"

class PhoneNumberTooLongException : public std::runtime_error {
    public:
    PhoneNumberTooLongException(const i18n::phonenumbers::PhoneNumber& number, const char* msg);

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
        MAX_COUNTRY_CODE = 999,
        MAX_LEADING_ZEROS = 15,
        //15 digits
        MAX_NATIONAL_NUMBER = 999999999999999,
    };

    enum : size_t {
        COUNTRY_CODE_BITS = 10,
        LEADING_ZEROS_BITS = 4,
        NATIONAL_NUMBER_BITS = 50,
    };

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

    operator i18n::phonenumbers::PhoneNumber() const;

    /*
     * Compares to another PhoneNumber using a fast collation heuristic
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

    google::protobuf::uint32 country_code() const {
        return getMasked(_data, COUNTRY_CODE_BITS, COUNTRY_CODE_OFFSET);
    }

    void country_code(google::protobuf::uint32 value) {
        _data = setMasked(_data, (google::protobuf::uint64)value, COUNTRY_CODE_BITS, COUNTRY_CODE_OFFSET);
    }

    google::protobuf::uint64 national_number() const {
        return getMasked(_data, NATIONAL_NUMBER_BITS, NATIONAL_NUMBER_OFFSET);
    }

    void national_number(google::protobuf::uint64 value) {
        _data = setMasked(_data, value, NATIONAL_NUMBER_BITS, NATIONAL_NUMBER_OFFSET);
    }

    google::protobuf::uint64 leading_zeros() const {
        return getMasked(_data, LEADING_ZEROS_BITS, LEADING_ZEROS_OFFSET);
    }

    void leading_zeros(google::protobuf::uint64 value) {
        _data = setMasked(_data, value, LEADING_ZEROS_BITS, LEADING_ZEROS_OFFSET);
    }

    private:
    google::protobuf::uint64 _data;
};
