#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

extern "C" {
	#include "postgres.h"
	#include "libpq/pqformat.h"
	#include "fmgr.h"
}

#include "error_handling.h"
#include "short_phone_number.h"

using namespace i18n::phonenumbers;

static const PhoneNumberUtil* const phoneUtil = PhoneNumberUtil::GetInstance();

/*
 * Utility functions
 */

static char* textToCString(const text* text) {
	size_t len = VARSIZE(text) - VARHDRSZ;
	char* str = (char*)palloc(len + 1);
	memcpy(str, VARDATA(text), len);
	str[len] = '\0';
	return str;
}

//Internal function used by phone_number_in and parse_phone_number
//TODO: take a std::string to minimize copying?
ShortPhoneNumber* parsePhoneNumber(const char* number_str, const char* country) throw() {
	try {
		PhoneNumber number;
		ShortPhoneNumber* short_number;

		short_number = (ShortPhoneNumber*)palloc0(sizeof(ShortPhoneNumber));
		if(short_number == nullptr) {
			throw std::bad_alloc();
		}

		PhoneNumberUtil::ErrorType error;
		error = phoneUtil->Parse(number_str, country, &number);
		if(error == PhoneNumberUtil::NO_PARSING_ERROR) {
			//Initialize short_number using placement new.
			new(short_number) ShortPhoneNumber(number);
			return short_number;
		} else {
			reportParseError(number_str, error);
			return nullptr;
		}
		//TODO: check number validity.
	} catch(const std::bad_alloc& e) {
		reportOutOfMemory();
	//TODO: figure out why we need this.
	} catch(const PhoneNumberTooLongException& e) {
		reportGenericError(e);
	} catch(const std::exception& e) {
		reportGenericError(e);
	}

	return nullptr;
}

//TODO: check null args (PG_ARGISNULL) and make non-strict?

/*
 * Extension functions
 */

extern "C" {
	#ifdef PG_MODULE_MAGIC
		PG_MODULE_MAGIC;
	#endif

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_in);

	PGDLLEXPORT Datum
	phone_number_in(PG_FUNCTION_ARGS) {
		const char *number_str = PG_GETARG_CSTRING(0);

		//TODO: use international format instead.
		ShortPhoneNumber* number = parsePhoneNumber(number_str, "US");
		if(number) {
			PG_RETURN_POINTER(number);
		} else {
			PG_RETURN_NULL();
		}
	}
	
	PGDLLEXPORT PG_FUNCTION_INFO_V1(parse_phone_number);

	PGDLLEXPORT Datum
	parse_phone_number(PG_FUNCTION_ARGS) {
		try {
			const text* number_text = PG_GETARG_TEXT_P(0);
			const text* country_text = PG_GETARG_TEXT_P(1);

			char* number_str = textToCString(number_text);
			char* country = textToCString(country_text);

			ShortPhoneNumber* number = parsePhoneNumber(number_str, country);
			//TODO: prevent leaks.
			pfree(number_str);
			pfree(country);
			if(number) {
				PG_RETURN_POINTER(number);
			} else {
				PG_RETURN_NULL();
			}
		} catch(std::bad_alloc& e) {
			reportOutOfMemory();
		} catch(std::exception& e) {
			reportGenericError(e);
		}
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_out);

	PGDLLEXPORT Datum
	phone_number_out(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* short_number = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			PhoneNumber number = *short_number;

			std::string formatted;
			phoneUtil->Format(number, PhoneNumberUtil::INTERNATIONAL, &formatted);

			//Copy the formatted number to a C-style string.
			//We must use the PostgreSQL allocator, not new/malloc.
			size_t len = formatted.length();
			char* result = (char*)palloc(len + 1);
			if(result == nullptr) {
				throw std::bad_alloc();
			}
			memcpy(result, formatted.data(), len);
			result[len] = '\0';

			PG_RETURN_CSTRING(result);
		} catch(const std::bad_alloc& e) {
			reportOutOfMemory();
		} catch (const std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_recv);

	PGDLLEXPORT Datum
	phone_number_recv(PG_FUNCTION_ARGS) {
		try {
			StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
			ShortPhoneNumber* number;

			number = (ShortPhoneNumber*)palloc(sizeof(ShortPhoneNumber));
			//TODO: make portable (fix endianness issues, etc.).
			pq_copymsgbytes(buf, (char*)number, sizeof(ShortPhoneNumber));
			PG_RETURN_POINTER(number);
		} catch(const std::bad_alloc& e) {
			reportOutOfMemory();
		} catch (const std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_send);

	PGDLLEXPORT Datum
	phone_number_send(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber *number = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			StringInfoData buf;

			pq_begintypsend(&buf);
			pq_sendbytes(&buf, (const char*)number, sizeof(ShortPhoneNumber));
			PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
		} catch(const std::bad_alloc& e) {
			reportOutOfMemory();
		} catch (const std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_equal);

	PGDLLEXPORT Datum
	phone_number_equal(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(*number1 == *number2);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_not_equal);

	PGDLLEXPORT Datum
	phone_number_not_equal(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(*number1 != *number2);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_less);

	PGDLLEXPORT Datum
	phone_number_less(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(number1->compare_fast(*number2) < 0);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_less_or_equal);

	PGDLLEXPORT Datum
	phone_number_less_or_equal(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(number1->compare_fast(*number2) <= 0);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_greater);

	PGDLLEXPORT Datum
	phone_number_greater(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(number1->compare_fast(*number2) > 0);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_greater_or_equal);

	PGDLLEXPORT Datum
	phone_number_greater_or_equal(PG_FUNCTION_ARGS) {
		try {
			const ShortPhoneNumber* number1 = (ShortPhoneNumber*)PG_GETARG_POINTER(0);
			const ShortPhoneNumber* number2 = (ShortPhoneNumber*)PG_GETARG_POINTER(1);

			PG_RETURN_BOOL(number1->compare_fast(*number2) >= 0);
		} catch(std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

}
