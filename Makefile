#The build configuration
CONFIG := release

EXTENSION := pg_libphonenumber
version := 1.0
extension_script := $(EXTENSION)--$(version).sql
DATA_built := $(extension_script)
REGRESS := regression

#Build options
cpp_files := $(wildcard *.cpp)

MODULE_big := pg_libphonenumber

OBJS := $(patsubst %.cpp,%.o,$(cpp_files))
PG_CPPFLAGS := -fPIC -std=c++11
ifeq ($(CONFIG),debug)
	PG_CPPFLAGS += -g -Og
else
	PG_CPPFLAGS += -O3
endif
SHLIB_LINK := -lphonenumber -lstdc++

#Clean options
EXTRA_CLEAN := $(extension_script) tools/get_sizeof_phone_number

#Load PGXS.
PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

#How to build the extension script:
$(extension_script): $(EXTENSION).sql.template tools/get_sizeof_phone_number
	sed "s/SIZEOF_PHONE_NUMBER/$(shell tools/get_sizeof_phone_number)/" $< > $@

tools/get_sizeof_phone_number: tools/get_sizeof_phone_number.cpp short_phone_number.h
	$(CXX) -std=c++11 $< -o $@

