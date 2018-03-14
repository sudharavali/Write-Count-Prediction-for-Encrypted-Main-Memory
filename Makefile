CC=g++
CFLAGS= -std=c++11 -W -Wall

cacheModel: cache_lru.cpp
	$(CC) $(CFLAGS) -o cacheModel cache_lru.cpp

clean:
	rm -rf cacheModel
