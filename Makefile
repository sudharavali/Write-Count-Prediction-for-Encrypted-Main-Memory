CC=g++
CFLAGS= -std=c++11 -W -Wall

all: cache_lru.cpp
	$(CC) $(CFLAGS) -g -o cacheModel cache_lru.cpp

debug: cache_lru.cpp
	$(CC) $(CFLAGS) -g -DDEBUG -o cacheModel cache_lru.cpp
clean:
	rm -rf cacheModel
