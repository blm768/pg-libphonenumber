#include "error_handling.h"

#include <typeinfo>
#include <string>

#include "phonenumbers/phonenumberutil.h"

extern "C" {
    #include "postgres.h"
}

#include "packed_phone_number.h"

using namespace i18n::phonenumbers;

namespace {
    const char* getParseErrorMessage(PhoneNumberUtil::ErrorType error) {
        using PNU = i18n::phonenumbers::PhoneNumberUtil;
        switch(error) {
        case PNU::NO_PARSING_ERROR:
            return "Parsed successfully";
        case PNU::INVALID_COUNTRY_CODE_ERROR:
            return "Invalid country code";
        case PNU::NOT_A_NUMBER:
            return "String does not appear to contain a phone number";
        case PNU::TOO_SHORT_AFTER_IDD:
            return "Too short after IDD";
        case PNU::TOO_SHORT_NSN:
            return "National number is too short";
        case PNU::TOO_LONG_NSN:
            return "National number is too long";
        default:
            //We have some generic parsing error.
            return "Unable to parse number";
        }
    }
}

/*
 * Utility functions for error handling
 */

void reportOutOfMemory() {
    ereport(ERROR,
        (errcode(ERRCODE_OUT_OF_MEMORY),
         errmsg("Out of memory")));
}

/*
 * Reports a generic C++ exception as a PostgreSQL error
 *
 * May produce specialized SQLSTATE values and/or messages
 * depending on the type of the exception
 */
void reportException(const std::exception& exception) {
    const std::bad_alloc* bad_alloc = dynamic_cast<const std::bad_alloc*>(&exception);
    if(bad_alloc != nullptr) {
        reportOutOfMemory();
        return;
    }

    const PhoneNumberTooLongException* too_long =
        dynamic_cast<const PhoneNumberTooLongException*>(&exception);
    if(too_long != nullptr) {
        std::string phone_number = too_long->number_string();
        phone_number += '\0';
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("phone number '%s' is too long", phone_number.data()),
                errdetail("%s", exception.what())));
        return;
    }

    //If we don't have a special way to handle this exception, report
    //a generic error.
    ereport(ERROR,
            (errcode(ERRCODE_EXTERNAL_ROUTINE_INVOCATION_EXCEPTION),
             errmsg("C++ exception: %s", typeid(exception).name()),
             errdetail("%s", exception.what())));
}

void reportParseError(const char* phone_number, PhoneNumberUtil::ErrorType err) {
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("unable to parse '%s' as a phone number", phone_number),
             errdetail("%s", getParseErrorMessage(err))));
}

void logInfo(const char* msg) {
    ereport(INFO,
            (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
            errmsg("%s", msg)));
}
