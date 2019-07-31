#pragma once

#include <exception>

#include "phonenumbers/phonenumberutil.h"

void reportOutOfMemory();
void reportException(const std::exception& exception);
void reportParseError(const char* phone_number, i18n::phonenumbers::PhoneNumberUtil::ErrorType err);

void logInfo(const char* msg);

