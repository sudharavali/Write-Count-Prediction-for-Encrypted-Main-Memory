#include "cache_lru.hpp"

const unsigned int CACHE_SIZE = 65535;
const int SET_NUM = 2;
const unsigned int SET_SIZE = CACHE_SIZE / SET_NUM;
const int WCHISTORY_SIZE = 2;
const int PWCBUFFER_SIZE = 1000;
const int PATTERNFIFO_SIZE = WCHISTORY_SIZE + 1;

void simplewcount::pushNew(const unsigned long& key, const unsigned int& value) {
   pwcBuffMap.insert(std::make_pair(key, value));
   pwcBuffQueue.push(key); 
}

void simplewcount::popOld() {
   unsigned long key = pwcBuffQueue.front();
   pwcBuffMap.erase(key);
   pwcBuffQueue.pop();
}

int main() {
   using namespace simplewcount;
   struct Cacheblock tempBlock;
   LruCache mycache[SET_NUM](SET_SIZE);   

   for (int j = 0; j < SET_NUM; ++j) {

	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		tempBlock.data = i;
		mycache[j].put(i,tempBlock);
	}

	if (mycache[j].size() == SET_SIZE)
		printf("Set: %d size: %d\n", j, mycache[j].size());
	else 
		printf("Set %d incorrect size", j);

	for (unsigned int i = 0; i < SET_SIZE-1; ++i) {
		mycache[j].get(i);
	}

	mycache[j].put(SET_SIZE+1, tempBlock);
	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		if(!mycache[j].exists(i)) {
			printf("Block %d not in set %d\n", i, j);
		}
	}

	for (unsigned int i = 1; i < SET_SIZE-1; ++i) {
		mycache[j].get(i);
	}

	mycache[j].put(SET_SIZE+2, tempBlock);
	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		if(!mycache[j].exists(i)) {
			printf("Block %d not in set %d\n", i, j);
		}
	}
	printf("Set: %d, Key: 5, Data: %d\n", j, mycache[j].get(5).data);
   }

   return 0;
}
