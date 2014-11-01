CC=g++
CXXFLAGS+=-std=gnu++14 -I.. -I../yesod

string_map_test: string_map_test.cpp
	$(CC) $(CXXFLAGS) -O3 -g -o $@ $^

pod_vector_test: pod_vector_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

sparse_vector_test: sparse_vector_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

p_array_test: p_array_test.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

t2: t2.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^