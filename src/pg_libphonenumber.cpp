#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

extern "C" {
#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"
}

#include "error_handling.h"
#include "packed_phone_number.h"
#include "phone_number.h"

using PhoneNumberUtil = i18n::phonenumbers::PhoneNumberUtil;
static const PhoneNumberUtil* const phoneUtil = PhoneNumberUtil::GetInstance();

/**
 * Clamps a value to the given (inclusive) range
 */
template<typename T> T clamp(const T& n, const T& lower, const T& upper) { return std::max(lower, std::min(n, upper)); }

/*
 * Utility functions
 */

/**
 * Converts a text object to a C-style string
 */
static char* text_to_c_string(const text* text) {
    size_t len = VARSIZE(text) - VARHDRSZ;
    char* str = (char*)palloc(len + 1);
    memcpy(str, VARDATA(text), len);
    str[len] = '\0';
    return str;
}

// Internal function used by phone_number_in and parse_phone_number
// TODO: take a std::string to minimize copying?
PhoneNumber* do_parse_phone_number(const char* number_str, const char* country) {
    i18n::phonenumbers::PhoneNumber number;

    PhoneNumberUtil::ErrorType error;
    error = phoneUtil->Parse(number_str, country, &number);
    if (error == PhoneNumberUtil::NO_PARSING_ERROR) {
        return PhoneNumber::make(number);
    } else {
        reportParseError(number_str, error);
        return nullptr;
    }
    // TODO: check number validity.
}

// Internal function used by packed_phone_number_in and
// parse_packed_phone_number
// TODO: take a std::string to minimize copying?
PackedPhoneNumber* do_parse_packed_phone_number(const char* number_str, const char* country) {
    i18n::phonenumbers::PhoneNumber number;
    PackedPhoneNumber* short_number = (PackedPhoneNumber*)palloc0(sizeof(PackedPhoneNumber));
    if (short_number == nullptr) {
        throw std::bad_alloc();
    }

    PhoneNumberUtil::ErrorType error;
    error = phoneUtil->Parse(number_str, country, &number);
    if (error == PhoneNumberUtil::NO_PARSING_ERROR) {
        // Initialize short_number using placement new.
        new (short_number) PackedPhoneNumber(number);
        return short_number;
    } else {
        reportParseError(number_str, error);
        return nullptr;
    }
    // TODO: check number validity.
}

// TODO: check null args (PG_ARGISNULL) and make non-strict?

/*
 * Extension functions
 */

extern "C" {
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
 * I/O functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_in);

PGDLLEXPORT Datum phone_number_in(PG_FUNCTION_ARGS) {
    try {
        const char* number_str = PG_GETARG_CSTRING(0);

        // TODO: use international format instead.
        PhoneNumber* number = do_parse_phone_number(number_str, "US");
        if (number) {
            PG_RETURN_POINTER(number);
        } else {
            PG_RETURN_NULL();
        }
    } catch (std::exception& e) {
        reportException(e);
        PG_RETURN_NULL();
    }
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_out);

PGDLLEXPORT Datum phone_number_out(PG_FUNCTION_ARGS) {
    try {
        const auto number = (const PhoneNumber*)PG_GETARG_POINTER(0);
        i18n::phonenumbers::PhoneNumber converted = *number;

        std::string formatted;
        phoneUtil->Format(converted, PhoneNumberUtil::INTERNATIONAL, &formatted);

        size_t len = formatted.length();
        char* result = (char*)palloc(len + 1);
        if (result == nullptr) {
            throw std::bad_alloc();
        }
        memcpy(result, formatted.data(), len);
        result[len] = '\0';

        PG_RETURN_CSTRING(result);
    } catch (const std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

/*
 * Operator functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_equal);

PGDLLEXPORT Datum phone_number_equal(PG_FUNCTION_ARGS) {
    auto number1 = (const PhoneNumber*)PG_GETARG_POINTER(0);
    auto number2 = (const PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 == *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 == *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_not_equal);

PGDLLEXPORT Datum phone_number_not_equal(PG_FUNCTION_ARGS) {
    auto number1 = (const PhoneNumber*)PG_GETARG_POINTER(0);
    auto number2 = (const PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 != *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 != *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_less);

PGDLLEXPORT Datum phone_number_less(PG_FUNCTION_ARGS) {
    const PhoneNumber* number1 = (PhoneNumber*)PG_GETARG_POINTER(0);
    const PhoneNumber* number2 = (PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 < *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 < *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_less_or_equal);

PGDLLEXPORT Datum phone_number_less_or_equal(PG_FUNCTION_ARGS) {
    const PhoneNumber* number1 = (PhoneNumber*)PG_GETARG_POINTER(0);
    const PhoneNumber* number2 = (PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 <= *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 <= *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_greater);

PGDLLEXPORT Datum phone_number_greater(PG_FUNCTION_ARGS) {
    const PhoneNumber* number1 = (PhoneNumber*)PG_GETARG_POINTER(0);
    const PhoneNumber* number2 = (PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 > *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 > *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_greater_or_equal);

PGDLLEXPORT Datum phone_number_greater_or_equal(PG_FUNCTION_ARGS) {
    const PhoneNumber* number1 = (PhoneNumber*)PG_GETARG_POINTER(0);
    const PhoneNumber* number2 = (PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(*number1 >= *number2), "must be noexcept");
    PG_RETURN_BOOL(*number1 >= *number2);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_cmp);

PGDLLEXPORT Datum phone_number_cmp(PG_FUNCTION_ARGS) {
    const PhoneNumber* number1 = (PhoneNumber*)PG_GETARG_POINTER(0);
    const PhoneNumber* number2 = (PhoneNumber*)PG_GETARG_POINTER(1);
    static_assert(noexcept(PhoneNumber::compare(*number1, *number2)), "must be noexcept");
    int64_t compared = PhoneNumber::compare(*number1, *number2);
    PG_RETURN_INT32(clamp<int64_t>(compared, -1, 1));
}

/*
 * I/O functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_in);

PGDLLEXPORT Datum packed_phone_number_in(PG_FUNCTION_ARGS) {
    try {
        const char* number_str = PG_GETARG_CSTRING(0);

        // TODO: use international format instead.
        PackedPhoneNumber* number = do_parse_packed_phone_number(number_str, "US");
        if (number) {
            PG_RETURN_POINTER(number);
        } else {
            PG_RETURN_NULL();
        }
    } catch (std::exception& e) {
        reportException(e);
        PG_RETURN_NULL();
    }
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_out);

PGDLLEXPORT Datum packed_phone_number_out(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* short_number = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        i18n::phonenumbers::PhoneNumber number = *short_number;

        std::string formatted;
        phoneUtil->Format(number, PhoneNumberUtil::INTERNATIONAL, &formatted);

        // Copy the formatted number to a C-style string.
        // We must use the PostgreSQL allocator, not new/malloc.
        size_t len = formatted.length();
        char* result = (char*)palloc(len + 1);
        if (result == nullptr) {
            throw std::bad_alloc();
        }
        memcpy(result, formatted.data(), len);
        result[len] = '\0';

        PG_RETURN_CSTRING(result);
    } catch (const std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_recv);

PGDLLEXPORT Datum packed_phone_number_recv(PG_FUNCTION_ARGS) {
    try {
        StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
        PackedPhoneNumber* number;

        number = (PackedPhoneNumber*)palloc(sizeof(PackedPhoneNumber));
        // TODO: make portable (fix endianness issues, etc.).
        pq_copymsgbytes(buf, (char*)number, sizeof(PackedPhoneNumber));
        PG_RETURN_POINTER(number);
    } catch (const std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_send);

PGDLLEXPORT Datum packed_phone_number_send(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        StringInfoData buf;

        pq_begintypsend(&buf);
        pq_sendbytes(&buf, (const char*)number, sizeof(PackedPhoneNumber));
        PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
    } catch (const std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

/*
 * Operator functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_equal);

PGDLLEXPORT Datum packed_phone_number_equal(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(*number1 == *number2);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_not_equal);

PGDLLEXPORT Datum packed_phone_number_not_equal(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(*number1 != *number2);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_less);

PGDLLEXPORT Datum packed_phone_number_less(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(number1->compare_fast(*number2) < 0);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_less_or_equal);

PGDLLEXPORT Datum packed_phone_number_less_or_equal(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(number1->compare_fast(*number2) <= 0);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_greater);

PGDLLEXPORT Datum packed_phone_number_greater(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(number1->compare_fast(*number2) > 0);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_greater_or_equal);

PGDLLEXPORT Datum packed_phone_number_greater_or_equal(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        PG_RETURN_BOOL(number1->compare_fast(*number2) >= 0);
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_cmp);

PGDLLEXPORT Datum packed_phone_number_cmp(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number1 = (PackedPhoneNumber*)PG_GETARG_POINTER(0);
        const PackedPhoneNumber* number2 = (PackedPhoneNumber*)PG_GETARG_POINTER(1);

        int64_t compared = number1->compare_fast(*number2);

        PG_RETURN_INT32(clamp<int64_t>(compared, -1, 1));
    } catch (std::exception& e) {
        reportException(e);
    }

    PG_RETURN_NULL();
}

/*
 * Other functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(parse_phone_number);

PGDLLEXPORT Datum parse_phone_number(PG_FUNCTION_ARGS) {
    try {
        const text* number_text = PG_GETARG_TEXT_P(0);
        const text* country_text = PG_GETARG_TEXT_P(1);

        char* number_str = text_to_c_string(number_text);
        char* country = text_to_c_string(country_text);

        PhoneNumber* number = do_parse_phone_number(number_str, country);
        pfree(number_str);
        pfree(country);
        if (number) {
            PG_RETURN_POINTER(number);
        } else {
            PG_RETURN_NULL();
        }
    } catch (std::exception& e) {
        reportException(e);
        PG_RETURN_NULL();
    }
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(parse_packed_phone_number);

PGDLLEXPORT Datum parse_packed_phone_number(PG_FUNCTION_ARGS) {
    try {
        const text* number_text = PG_GETARG_TEXT_P(0);
        const text* country_text = PG_GETARG_TEXT_P(1);

        char* number_str = text_to_c_string(number_text);
        char* country = text_to_c_string(country_text);

        PackedPhoneNumber* number = do_parse_packed_phone_number(number_str, country);
        pfree(number_str);
        pfree(country);
        if (number) {
            PG_RETURN_POINTER(number);
        } else {
            PG_RETURN_NULL();
        }
    } catch (std::exception& e) {
        reportException(e);
        PG_RETURN_NULL();
    }
}

/*
 * General functions
 */

PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_country_code);

PGDLLEXPORT Datum phone_number_country_code(PG_FUNCTION_ARGS) {
    const PhoneNumber* number = (PhoneNumber*)PG_GETARG_POINTER(0);
    static_assert(noexcept(number->country_code()), "must be noexcept");
    PG_RETURN_INT32(number->country_code());
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(packed_phone_number_country_code);

PGDLLEXPORT Datum packed_phone_number_country_code(PG_FUNCTION_ARGS) {
    try {
        const PackedPhoneNumber* number = (PackedPhoneNumber*)PG_GETARG_POINTER(0);

        PG_RETURN_INT32(number->country_code());
    } catch (std::exception& e) {
        reportException(e);
        PG_RETURN_NULL();
    }
}
}
