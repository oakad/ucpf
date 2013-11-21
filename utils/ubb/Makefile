JS_CFLAGS=$(shell pkg-config --cflags mozjs-17.0)
JS_LIBS=$(shell pkg-config --libs mozjs-17.0)


ubb : ubb.cpp
	g++ -std=c++11 $(JS_CFLAGS) $(JS_LIBS) -o $@ $^
