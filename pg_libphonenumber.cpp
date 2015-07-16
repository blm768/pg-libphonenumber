#include <exception>
#include <string>

#include "phonenumbers/phonenumberutil.h"

extern "C" {
	#include "postgres.h"
	#include "libpq/pqformat.h"
	#include "fmgr.h"
}

using namespace i18n::phonenumbers;

PhoneNumberUtil *phoneUtil = PhoneNumberUtil::GetInstance();

//Raised when a phone number can't be parsed
class ParseException : std::exception {
	public:
	ParseException(PhoneNumberUtil::ErrorType et) : error_type(et) {}

	const char* what() const throw() override {
		using PNU = i18n::phonenumbers::PhoneNumberUtil;
		switch(error_type) {
		case PNU::NO_PARSING_ERROR:
			//This case shouldn't occur in correctly-functioning code.
			assert(0);
			return "Parsed successfully";
		case PNU::NOT_A_NUMBER:
			return "String does not appear to contain a phone number.";
		case PNU::INVALID_COUNTRY_CODE_ERROR:
			return "Invalid country code";
		//TODO: handle more error cases specifically.
		default:
			//We have some generic parsing error.
			return "Unrecognized number format";
		}
	};

	private:
	PhoneNumberUtil::ErrorType error_type;
};

//Utility functions for error handling

void reportOutOfMemory() {
	ereport(ERROR,
		(errcode(ERRCODE_OUT_OF_MEMORY)));
}

void reportParseError(const char* phone_number, ParseException& exception) {
	ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unable to parse '%s' as a phone number", phone_number),
			 errdetail("%s", exception.what())));
}

void reportGenericError(std::exception& exception) {
	ereport(ERROR,
			(errcode(ERRCODE_EXTERNAL_ROUTINE_INVOCATION_EXCEPTION),
			 errmsg("C++ exception: %s", typeid(exception).name()),
			 errdetail("%s", exception.what())));
}

void logInfo(const char* msg) {
	ereport(INFO,
			(errcode(ERRCODE_SUCCESSFUL_COMPLETION),
			errmsg("%s", msg)));
}

/*
 * Utility functions
 */

//Internal function used by phone_number_in and parse_phone_number
PhoneNumber* parsePhoneNumber(const char* number_str, const char* country) {
	PhoneNumber *number;

	number = (PhoneNumber*)palloc0(sizeof(PhoneNumber));
	//TODO: can this be removed? (palloc normally handles this, right?)
	if(number == nullptr) {
		throw std::bad_alloc();
	}
	new(number) PhoneNumber();

	PhoneNumberUtil::ErrorType error;
	error = phoneUtil->Parse(number_str, country, number);
	if(error != PhoneNumberUtil::NO_PARSING_ERROR) {
		throw ParseException(error);
	}
	//TODO: check number validity.
	return number;
}

//TODO: handle non-exception thrown types? (shouldn't happen, but you never know...)
//TODO: check null args? (PG_ARGISNULL). Make non-strict?

/*
 * Extension functions
 */

extern "C" {
	#ifdef PG_MODULE_MAGIC
		PG_MODULE_MAGIC;
	#endif

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_in);

	PGDLLEXPORT
	Datum
	phone_number_in(PG_FUNCTION_ARGS)
	{
		const char *number_str = PG_GETARG_CSTRING(0);

		try {
			PG_RETURN_POINTER(parsePhoneNumber(number_str, "US"));
		} catch(std::bad_alloc& e) {
			reportOutOfMemory();
		} catch(ParseException e) {
			reportParseError(number_str, e);
		} catch (std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}
	
	PGDLLEXPORT PG_FUNCTION_INFO_V1(parse_phone_number);

	PGDLLEXPORT
	Datum
	parse_phone_number(PG_FUNCTION_ARGS)
	{
		const char *number_str = PG_GETARG_CSTRING(0);
		const char *country = PG_GETARG_CSTRING(1);

		try {
			PG_RETURN_POINTER(parsePhoneNumber(number_str, country));
		} catch(std::bad_alloc& e) {
			reportOutOfMemory();
		} catch(ParseException e) {
			reportParseError(number_str, e);
		} catch (std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_out);

	PGDLLEXPORT
	Datum
	phone_number_out(PG_FUNCTION_ARGS)
	{
		try {
			const PhoneNumber *number = (PhoneNumber*)PG_GETARG_POINTER(0);
			std::string formatted;
			char *result;

			phoneUtil->Format(*number, PhoneNumberUtil::INTERNATIONAL, &formatted);

			//Copy the formatted number to a C-style string.
			//We must use the PostgreSQL allocator, not new/malloc.
			size_t len = formatted.length();
			result = (char*)palloc(len + 1);
			if(result == nullptr) {
				reportOutOfMemory();
				PG_RETURN_NULL();
			}
			memcpy(result, formatted.data(), len);
			result[len] = '\0';

			PG_RETURN_CSTRING(result);
		} catch(std::bad_alloc& e) {
			reportOutOfMemory();
		} catch (std::exception& e) {
			reportGenericError(e);
		}

		PG_RETURN_NULL();
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_recv);

	//TODO: handle leading zeroes?
	PGDLLEXPORT
	Datum
	phone_number_recv(PG_FUNCTION_ARGS)
	{
		StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
		PhoneNumber *result;

		//TODO: error handling?
		result = (PhoneNumber*)palloc0(sizeof(PhoneNumber));
		result->set_country_code(pq_getmsgint(buf, 4));
		result->set_national_number(pq_getmsgint64(buf));
		PG_RETURN_POINTER(result);
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_send);

	PGDLLEXPORT
	Datum
	phone_number_send(PG_FUNCTION_ARGS)
	{
		PhoneNumber *number = (PhoneNumber*)PG_GETARG_POINTER(0);
		StringInfoData buf;

		pq_begintypsend(&buf);
		pq_sendint(&buf, number->country_code(), 4);
		pq_sendint64(&buf, number->national_number());
		PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
	}
}
