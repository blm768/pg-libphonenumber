EXTENSION := pg_libphonenumber
version := 1.0
extension_script := $(EXTENSION)--$(version).sql
DATA_built := $(extension_script)

MODULE_big := pg_libphonenumber
OBJS := pg_libphonenumber.o short_phone_number.o
PG_CPPFLAGS := -fPIC -std=c++11 -g
SHLIB_LINK := -lphonenumber -lstdc++

EXTRA_CLEAN := $(extension_script) get_sizeof_phone_number

PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

$(extension_script): $(EXTENSION).sql.template get_sizeof_phone_number
	sed "s/SIZEOF_PHONE_NUMBER/$(shell ./get_sizeof_phone_number)/" $< > $@

get_sizeof_phone_number: get_sizeof_phone_number.cpp
	$(CXX) -std=c++11 $< -o $@
