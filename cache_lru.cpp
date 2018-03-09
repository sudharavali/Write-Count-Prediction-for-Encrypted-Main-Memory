#include "cache_lru.hpp"

void simplewcount::testCache() {

   struct Cacheblock tempBlock {};
   for (int j = 0; j < SET_NUM; ++j) {

 	std::cout << "SET: "  << j << std::endl;
	
	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		tempBlock.data = i;
		mycache[j].put(i,tempBlock);
	}

	if (mycache[j].size() == SET_SIZE) {
  	 	std::cout << "Set size "  << mycache[j].size() << std::endl;
	} else { 
		std::cout << "Incorrect set size"  << std::endl;
	}

	for (unsigned int i = 0; i < SET_SIZE-1; ++i) {
		mycache[j].get(i);
	}

	mycache[j].put(SET_SIZE+1, tempBlock);
	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		if(!mycache[j].exists(i)) {
			std::cout << "Block " << i << " not in set" << std::endl;
		}
	}

	for (unsigned int i = 1; i < SET_SIZE-1; ++i) {
		mycache[j].get(i);
	}

	mycache[j].put(SET_SIZE+2, tempBlock);
	for (unsigned int i = 0; i < SET_SIZE; ++i) {
		if(!mycache[j].exists(i)) {
			std::cout << "Block " << i << " not in set" << std::endl;
		}
	}
	std::cout << "Key: 5, Data: " << mycache[j].get(5).data << std::endl;
   }

}

void simplewcount::pushNew(const unsigned long& key, const unsigned int& value) {
   pwcBuffMap.insert(std::make_pair(key, value));
   pwcBuffQueue.push(key); 
}

void simplewcount::popOld() {
   unsigned long key = pwcBuffQueue.front();
   pwcBuffMap.erase(key);
   pwcBuffQueue.pop();
}

void simplewcount::fillMainMemory(std::string filename) {

   unsigned long cacheLine;
   std::string instrAddr, memOp, memAddr;   
   struct Cacheblock tempBlock = {};
   std::ifstream infile(filename);
   std::string fileLine;
   while (std::getline(infile, fileLine)) {
        std::istringstream traces(fileLine);
        traces >> instrAddr >> memOp >> memAddr;
        
  	std::cout << "Bytr address "  << memAddr << std::endl;

        cacheLine = std::stoul(memAddr, nullptr, 16) / 64;
        std::cout << "Cache line " << cacheLine << std::endl;
	
	mainMemory[cacheLine] = tempBlock;
   }
   std::cout << "# of unique lines: " << mainMemory.size() <<std::endl;
} 


int main() {
   using namespace simplewcount;
   
   fillMainMemory(FILE_NAME);
   testCache();

   return 0;
}
