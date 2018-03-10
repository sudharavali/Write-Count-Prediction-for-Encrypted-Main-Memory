#include "cache_lru.hpp"


void simplewcount::pushNew(const addr_t& key, const int& value) {
   pwcBuffMap.insert(std::make_pair(key, value));
   pwcBuffQueue.push(key); 
}

void simplewcount::popOld() {
   addr_t key = pwcBuffQueue.front();
   pwcBuffMap.erase(key);
   pwcBuffQueue.pop();
}

void simplewcount::fillMainMemory(std::string filename) {

   addr_t cacheLine;
   std::string instrAddr, memOp, memAddr;   
   struct Cacheblock tempBlock = {};

   std::ifstream infile(filename);
   std::string fileLine;
   while (std::getline(infile, fileLine)) {
        std::istringstream traces(fileLine);
        traces >> instrAddr >> memOp >> memAddr;  

        cacheLine = std::stoul(memAddr, nullptr, 16) / 64;

	tempBlock.wcActual = cacheLine % 10;	
	mainMemory[cacheLine] = tempBlock;
   }
} 

int simplewcount::getActualwc(const addr_t& key) {
   
   auto it = mainMemory.find(key);
   if (it == mainMemory.end()) {
   	throw std::range_error("NOT FOUND IN MM");		
   }
   else {
	return it->second.wcActual; 
   }

}



int main() {
   using namespace simplewcount;
   
   fillMainMemory(FILE_NAME);

   std::ifstream infile(FILE_NAME);
   std::string fileLine;
  
   while (std::getline(infile, fileLine)) {
        
   	std::string instrAddr, memOp, memAddr;   
	std::istringstream traces(fileLine);
        traces >> instrAddr >> memOp >> memAddr;

	//TODO get set bits
   	unsigned long cacheLine;
	cacheLine = std::stoul(memAddr, nullptr, 16) / 64;
  	int cacheSet = 0;

	//Check if exists in cache
	if(!mycache[cacheSet].exists(cacheLine)) {
		int actualWc = getActualwc(cacheLine);	
	}

	
	
   }

   return 0;
}
