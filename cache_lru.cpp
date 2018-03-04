#include "cache_lru.hpp"

const unsigned int CACHE_SIZE = 65535;

int main() {
   simplecache::LruCache mycache(CACHE_SIZE);

   for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
   	mycache.put(i,i);
   }

   if (mycache.size() == CACHE_SIZE)
  	 printf("Cache size: %d\n", mycache.size());
   else 
	printf("WRONG CACHE SIZE");
   
   for (unsigned int i = 0; i < CACHE_SIZE-1; ++i) {
   	mycache.get(i);
   }
   mycache.put(CACHE_SIZE+1, 7);
   for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
	if(!mycache.exists(i)) {
		printf("%d, NOT IN CACHE\n", i);
	}
   }

   for (unsigned int i = 1; i < CACHE_SIZE-1; ++i) {
   	mycache.get(i);
   }
   mycache.put(CACHE_SIZE+2, 7);
   for (unsigned int i = 0; i < CACHE_SIZE; ++i) {
	if(!mycache.exists(i)) {
		printf("%d, NOT IN CACHE\n", i);
	}
   }
   return 0;
}
