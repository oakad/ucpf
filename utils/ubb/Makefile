JS_CFLAGS     = $(shell pkg-config --cflags mozjs-17.0)
JS_LIBS       = $(shell pkg-config --libs mozjs-17.0)
SQLITE_CFLAGS = $(shell pkg-config --cflags sqlite3)
SQLITE_LIBS   = $(shell pkg-config --libs sqlite3)

CXXFLAGS += -std=c++11 -pthread -I.
CXXFLAGS += $(JS_CFLAGS) $(SQLITE_CFLAGS)
LIBS   += $(JS_LIBS) $(SQLITE_LIBS)

CXX = g++

ubb : ubb.cpp sqlite.cpp memfd.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $^

sqlite.o : sqlite.hpp memfd.hpp

memfd.o : memfd.hpp

ubb_base.hpp : ubb_base.bin
	./bin2cstr.pl ubb_base_data < $< > $@

ubb_base.bin : ubb_schema.sql
	rm -f $@
	sqlite3 $@ -init $< .quit ; true
