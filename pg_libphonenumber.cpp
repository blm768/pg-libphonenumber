#include <exception>
#include <string>

#define I18N_PHONENUMBERS_USE_BOOST
#include "phonenumbers/phonenumberutil.h"

extern "C" {
	#include "postgres.h"
	#include "libpq/pqformat.h"
	#include "fmgr.h"
}

using namespace i18n::phonenumbers;

PhoneNumberUtil *phoneUtil = PhoneNumberUtil::GetInstance();

//Utility functions for error handling

void reportOutOfMemory() {
	ereport(ERROR,
		(errcode(ERRCODE_OUT_OF_MEMORY)));
}

void reportGenericError(std::exception& exception) {
	ereport(ERROR,
			(errcode(ERRCODE_EXTERNAL_ROUTINE_INVOCATION_EXCEPTION),
			 errmsg("%s", exception.what())));
}

//TODO: handle non-exception thrown types? (shouldn't happen, but you never know...)

extern "C" {
	#ifdef PG_MODULE_MAGIC
		PG_MODULE_MAGIC;
	#endif

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_in);

	PGDLLEXPORT
	Datum
	phone_number_in(PG_FUNCTION_ARGS)
	{
		char *str = PG_GETARG_CSTRING(0);
		PhoneNumber *number;

		number = (PhoneNumber*)palloc0(sizeof(PhoneNumber));
		if(number == nullptr) {
			reportOutOfMemory();
		}

		try {
			PhoneNumberUtil::ErrorType error;	
			error = phoneUtil->Parse(str, "US", number);
			if(error == PhoneNumberUtil::NO_PARSING_ERROR) {
				//The number was parsed correctly; return it.
				PG_RETURN_POINTER(number);
			}
			//TODO: handle errors.
		} catch(std::bad_alloc& e) {
			reportOutOfMemory();
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
		PhoneNumber *number = (PhoneNumber*)PG_GETARG_POINTER(0);
		std::string formatted;
		char *result;

		try {
			phoneUtil->Format(*number, PhoneNumberUtil::INTERNATIONAL, &formatted);
		} catch (std::exception e) {
			//TODO: implement.
		}

		size_t len = formatted.length();
		ereport(INFO,
				(errcode(ERRCODE_SUCCESSFUL_COMPLETION),
				errmsg("Length: %zu", len)));
		result = (char*)palloc(len + 1);
		//TODO: test allocation.
		memcpy(result, formatted.data(), len);
		result[len] = '\0';

		PG_RETURN_CSTRING(result);
	}

	PGDLLEXPORT PG_FUNCTION_INFO_V1(phone_number_recv);

	PGDLLEXPORT
	Datum
	phone_number_recv(PG_FUNCTION_ARGS)
	{
		StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
		PhoneNumber *result;

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
