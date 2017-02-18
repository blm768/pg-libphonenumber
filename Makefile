# The build configuration (debug or release)
CONFIG := release

# Extension packaging options
EXTENSION := pg_libphonenumber
version := 0.1.0
extension_script := $(EXTENSION)--$(version).sql
DATA_built := $(extension_script)
REGRESS := regression

# Build options
cpp_files := $(wildcard src/*.cpp)

# The native module to build
MODULE_big := pg_libphonenumber
# The object files that go into the module
OBJS := $(patsubst %.cpp,%.o,$(cpp_files))

# C flags
PG_CPPFLAGS := -fPIC -std=c++11 -Isrc/
ifeq ($(CONFIG),debug)
	PG_CPPFLAGS += -g -Og
else
	PG_CPPFLAGS += -O3
endif
# Extra libraries to link
SHLIB_LINK := -lphonenumber -lstdc++

# Clean options
EXTRA_CLEAN := $(extension_script) tools/get_sizeof_phone_number

# Load PGXS.
PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# Build the extension script:
$(extension_script): $(EXTENSION).sql.template tools/get_sizeof_phone_number
	sed "s/SIZEOF_PHONE_NUMBER/$(shell tools/get_sizeof_phone_number)/" $< > $@

# Gets the size of a ShortPhoneNumber (which corresponds to the phone_number type in SQL)
tools/get_sizeof_phone_number: tools/get_sizeof_phone_number.cpp src/short_phone_number.h
	$(CXX) $(PG_CPPFLAGS) $< -o $@
