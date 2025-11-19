CC := g++
CFLAGS := -fPIC -c

.PHONY: all clean

all: libhttp.so

libhttp.so: HTTPResponse.o HTTPRequest.o
	$(CC) -shared HTTPResponse.o HTTPRequest.o -o /usr/local/lib/libhttp.so

HTTPRequest.o: HTTPRequest.cpp
	$(CC) $(CFLAGS) HTTPRequest.cpp -o HTTPRequest.o

HTTPResponse.o: HTTPResponse.cpp
	$(CC) $(CFLAGS) HTTPResponse.cpp -o HTTPResponse.o

clean:
	rm -f *.o libhttp.so