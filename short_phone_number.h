#include <exception>

#include "phonenumbers/phonenumberutil.h"

#include "mask.h"

class PhoneNumberTooLongException : public std::runtime_error {
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
	
	enum : size_t {
		COUNTRY_CODE_BITS = 10,
		NATIONAL_NUMBER_BITS = 50,
		LEADING_ZEROS_BITS = 4,
	};
	
	enum : size_t {
		COUNTRY_CODE_OFFSET = 0,
		NATIONAL_NUMBER_OFFSET = COUNTRY_CODE_OFFSET + COUNTRY_CODE_BITS,
		LEADING_ZEROS_OFFSET = NATIONAL_NUMBER_OFFSET + NATIONAL_NUMBER_BITS,
	};

	ShortPhoneNumber(i18n::phonenumbers::PhoneNumber number);

	operator i18n::phonenumbers::PhoneNumber() const;

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

