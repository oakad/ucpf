CC=g++
CXXFLAGS+=-std=gnu++14 -I.. -I../../yesod

t6 : t6.cpp
	$(CC) $(CXXFLAGS) -g -o $@ $^

test_fwr : test_fwr.cpp
	$(CC) $(CXXFLAGS) -g -O3 -lquadmath -o $@ $^

test_frd : test_frd.cpp
	$(CC) $(CXXFLAGS) -Wformat=0 -g -O3 -lquadmath -o $@ $^

t5 : t5.cpp
	$(CC) $(CXXFLAGS) -m32 -O3 -o $@ $^

t4 : t4.cpp
	$(CC) $(CXXFLAGS) -O3 -lquadmath -o $@ $^

t3 : t3.cpp
	$(CC) $(CXXFLAGS) -o $@ $^

t1 : t1.cpp
	$(CC) $(CXXFLAGS) -o $@ $^

magic_div : magic_div.cpp
	$(CC) $(CXXFLAGS) -o $@ $^

pow_5 : pow_5.cpp
	$(CC) $(CXXFLAGS) -lmpfr -lgmp -o $@ $^
