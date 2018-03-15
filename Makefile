CC=g++
CFLAGS= -std=c++11 -W -Wall

all: cache_lru.cpp
	$(CC) $(CFLAGS) -o cacheModel cache_lru.cpp

debug: cache_lru.cpp
	$(CC) $(CFLAGS) -DDEBUG -o cacheModel cache_lru.cpp
clean:
	rm -rf cacheModel
