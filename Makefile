CC=g++
CFLAGS= -std=c++11

cacheModel: cache_lru.cpp
	$(CC) $(CFLAGS) -o cacheModel cache_lru.cpp

clean:
	rm -rf cacheModel
