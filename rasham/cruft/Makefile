CC=clang++
CXXFLAGS=-std=c++11 -O3

main : rasham-z librasham-y.so rasham-x

rasham-x : rasham-x.cpp
	$(CC) $(CXXFLAGS) -L. -lrasham-y -ldl -o $@ $^

librasham-y.so : rasham-y.cpp
	$(CC) $(CXXFLAGS) -o $@ -shared -fPIC -ldl $^

rasham-z : rasham-z.cpp
	$(CC) $(CXXFLAGS) -ldl -o $@ $^