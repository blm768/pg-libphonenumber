#include <exception>

#include "phonenumbers/phonenumberutil.h"

class PhoneNumberTooLongException : std::runtime_error {
	public:
	PhoneNumberTooLongException(const i18n::phonenumbers::PhoneNumber& number, const char* msg);

	private:
	i18n::phonenumbers::PhoneNumber _number;

	static const i18n::phonenumbers::PhoneNumberUtil* const phoneUtil; 
};

class ShortPhoneNumber {
	public:
	enum : size_t {
		MAX_COUNTRY_CODE = 999,
		//15 digits
		MAX_NATIONAL_NUMBER = 999999999999999,
		MAX_LEADING_ZEROS = 15,
	};

	ShortPhoneNumber(i18n::phonenumbers::PhoneNumber number);

	operator i18n::phonenumbers::PhoneNumber() const;

	private:
	google::protobuf::uint32 _country_code : 10;
	google::protobuf::uint64 _national_number : 50;
	google::protobuf::uint32 _leading_zeros : 4;
};

