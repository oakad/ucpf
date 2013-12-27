JS_CFLAGS     = $(shell pkg-config --cflags mozjs-17.0)
JS_LIBS       = $(shell pkg-config --libs mozjs-17.0)
SQLITE_CFLAGS = $(shell pkg-config --cflags sqlite3)
SQLITE_LIBS   = $(shell pkg-config --libs sqlite3)

CXXFLAGS += -std=c++11 -g -pthread -I.
LDFLAGS += -std=c++11 -g -pthread
CXXFLAGS += $(JS_CFLAGS) $(SQLITE_CFLAGS)
LIBS   += $(JS_LIBS) $(SQLITE_LIBS)

CXX = g++

OBJ = ubb.o sqlite.o js_sqlite.o memfd.o ubb_base.o

ubb : $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) -o $@ $^

memfd.o : memfd.hpp

sqlite.o : sqlite.hpp memfd.hpp

js_sqlite.o : js_sqlite.hpp

ubb_base.cpp : ubb_base.bin
	./bin2cstr.pl ubb_base_data < $< > $@

ubb_base.bin : ubb_schema.sql
	rm -f $@
	sqlite3 $@ -init $< .quit ; true
