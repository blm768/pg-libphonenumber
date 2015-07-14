MODULE_big := pg_libphonenumber
OBJS := pg_libphonenumber.o
PG_CPPFLAGS := -fPIC -std=c++11
SHLIB_LINK := -lphonenumber -lstdc++

PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
