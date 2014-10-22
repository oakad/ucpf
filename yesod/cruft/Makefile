CC=g++
CXXFLAGS+=-std=gnu++1y -fabi-version=8 -I.. -I../yesod

sparse_vector_test: sparse_vector_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

p_array_test: p_array_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

string_map_test: string_map_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

t2: t2.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^