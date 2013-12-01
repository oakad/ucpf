JS_CFLAGS     = $(shell pkg-config --cflags mozjs-17.0)
JS_LIBS       = $(shell pkg-config --libs mozjs-17.0)
SQLITE_CFLAGS = $(shell pkg-config --cflags sqlite3)
SQLITE_LIBS   = $(shell pkg-config --libs sqlite3)

CXXFLAGS += -std=c++11 -pthread
CXXFLAGS += $(JS_CFLAGS) $(SQLITE_CFLAGS)
LIBS   += $(JS_LIBS) $(SQLITE_LIBS)



ubb : ubb.cpp sqlite.cpp
	g++ $(CXXFLAGS) $(LIBS) -o $@ $^

sqlite.cpp : ubb_base.hpp

ubb_base.hpp : ubb_base.bin
	./bin2cstr.pl ubb_base_data < $< > $@

ubb_base.bin : ubb_schema.sql
	rm -f $@
	sqlite3 $@ -init $< .quit ; true
