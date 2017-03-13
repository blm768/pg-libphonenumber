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
PG_CPPFLAGS := -fPIC -std=c++11 -Isrc/ -I/usr/include
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

# Docker stuff
.PHONY: docker-image

docker-image: clean
	docker build -t pg_libphonenumber .
