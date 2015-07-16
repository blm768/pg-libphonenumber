#include <iostream>

#include "phonenumbers/phonenumber.pb.h"

using namespace i18n::phonenumbers;

int main(int argc, const char** argv) {
	std::cout << sizeof(PhoneNumber) << std::endl;
	return 0;
}
