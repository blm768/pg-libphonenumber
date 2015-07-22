#include <exception>

#include "phonenumbers/phonenumberutil.h"

void reportOutOfMemory();
void reportParseError(const char* phone_number, i18n::phonenumbers::PhoneNumberUtil::ErrorType err);
void reportGenericError(const std::exception& exception);

void logInfo(const char* msg);

