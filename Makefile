# The build configuration (debug or release)
CONFIG := release

# Extension packaging options
EXTENSION := pg_libphonenumber
DATA := sql/$(EXTENSION)--*.sql
REGRESS := regression

# Build options
cpp_files := $(wildcard src/*.cpp)

# The native module to build
MODULE_big := pg_libphonenumber
# The object files that go into the module
OBJS := $(patsubst %.cpp,%.o,$(cpp_files))

# C flags
PG_CPPFLAGS := -fPIC -std=gnu++14
PG_CPPFLAGS += -Isrc/ -I/usr/include
PG_CPPFLAGS += -Wall -Wextra
ifeq ($(CONFIG),debug)
	PG_CPPFLAGS += -g -Og
else
	PG_CPPFLAGS += -O3
endif
# Extra libraries to link
SHLIB_LINK := -lphonenumber -lstdc++

# Load PGXS.
PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
