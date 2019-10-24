#include "packed_phone_number.h"

using namespace google::protobuf;
using namespace i18n::phonenumbers;

#include "error_handling.h"

namespace {
const PhoneNumberUtil* phoneUtil = PhoneNumberUtil::GetInstance();
}

PackedPhoneNumber::PackedPhoneNumber(const i18n::phonenumbers::PhoneNumber& number) {
    // TODO: check has_national_number(), etc?
    uint32 country_code = number.country_code();
    if (country_code > max_country_code) {
        throw PhoneNumberTooLongException(number, "Country code is too long");
    }
    this->country_code(country_code);

    uint64_t national_number = number.national_number();
    if (national_number > max_national_number) {
        throw PhoneNumberTooLongException(number, "National number is too long");
    }
    this->national_number(national_number);

    if (number.has_number_of_leading_zeros()) {
        uint32 leading_zeros = number.number_of_leading_zeros();
        if (leading_zeros > max_leading_zeros) {
            throw PhoneNumberTooLongException(number, "Too many leading zeros");
        }
        this->leading_zeros(leading_zeros);
    } else {
        this->leading_zeros(0);
    }
}

PackedPhoneNumber::operator PhoneNumber() const {
    PhoneNumber number;
    number.set_country_code(country_code());
    number.set_national_number(national_number());
    int32 leading_zeros = this->leading_zeros();
    number.set_italian_leading_zero(leading_zeros > 0);
    number.set_number_of_leading_zeros(leading_zeros);
    return number;
}
