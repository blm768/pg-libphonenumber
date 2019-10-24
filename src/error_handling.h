#pragma once

#include <exception>

#include "phonenumbers/phonenumberutil.h"

/**
 * Raised when a phone number is too long to fit in the target phone number type.
 *
 * This shouldn't happen for any phone numbers that meet ITU specifications.
 */
class PhoneNumberTooLongException : public std::runtime_error {
  public:
    PhoneNumberTooLongException(const i18n::phonenumbers::PhoneNumber& number, const char* msg);

    /// Returns the original phone number object
    const i18n::phonenumbers::PhoneNumber& number() const { return _number; }

    // TODO: just get the number string from which the PhoneNumber was parsed?
    // (if it exists...)
    std::string number_string() const;

  private:
    i18n::phonenumbers::PhoneNumber _number;
};

void reportOutOfMemory();
void reportException(const std::exception& exception);
void reportParseError(const char* phone_number, i18n::phonenumbers::PhoneNumberUtil::ErrorType err);

void logInfo(const char* msg);
