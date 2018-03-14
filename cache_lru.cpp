#include "cache_lru.hpp"


void simplewcount::putinCache(const int& set, const addr_t& key, const struct Cacheblock& block) {
	
   if (mycache[set].size() >= SET_SIZE) {
	addr_t evictKey = mycache[set].old();
	struct Cacheblock evictBlock = mycache[set].getOld(evictKey);
	if (evictBlock.isDirty) {
		evictBlock.isDirty = 0;
		evictBlock.wcActual++;
		//Writeback E to MM
		mainMemory[evictKey] = evictBlock;
	}
   }
   //Put A in Cache
   mycache[set].put(key, block);

}

void simplewcount::updateDirty(const std::string& op, const int& set, const addr_t& key) {
   //Update LRU
   struct Cacheblock tempBlock = mycache[set].get(key);
   if (op == "W") {
   	tempBlock.isDirty = 1;
   }
   //Even in case of READ we need to update LRU
   mycache[set].put(key, tempBlock);
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

	mainMemory[cacheLine] = tempBlock;
   }
   std::cout << "# of unique lines: " << mainMemory.size() << std::endl << std::endl;
} 

void simplewcount::printMainMemory() {
   for (auto it : mainMemory) {
	std::cout << it.first << "| wcA: " << it.second.wcActual << ", wcH: ";
	for (int i = 0; i < WCHISTORY_SIZE; ++i) {
		std::cout << it.second.wcHistory[i] << " ";
	}
   std::cout << std::endl;
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

struct simplewcount::Cacheblock simplewcount::getBlock(const addr_t& key) {
   
   struct Cacheblock tempBlock = {};
   auto it = mainMemory.find(key);
   if (it == mainMemory.end()) {
   	throw std::range_error("NOT FOUND IN MM");		
   }
   else {
	tempBlock = it->second;
	return tempBlock;
   }

}

void simplewcount::printPattern() {
   std::cout << "Pattern buffer" << std::endl;
   for (auto it = patternBuffQueue.cbegin(); it != patternBuffQueue.cend(); ++it) {	
	std::cout <<  it->first << ", " << it->second << std::endl;
   } 
	
}

void simplewcount::updatePattern(const addr_t& key, const int& wc) {
   //Check that patter buffer is full
   if (patternBuffQueue.size() == PATTERNFIFO_SIZE) {
	//Get last block
	std::pair<addr_t, int> last = patternBuffQueue.back();

	patternBuffQueue.pop_back();

	//Find set
	int set = 0;
	if (mycache[set].exists(last.first)) {
		struct Cacheblock tempBlock = mycache[set].getOld(last.first);
		int i = 0;
		for (auto it = patternBuffQueue.crbegin(); it != patternBuffQueue.crend(); ++it) {	
			tempBlock.wcHistory[i] = it->second;
                        i++;
			//TODO Ask if we update dirty status.
			tempBlock.isDrity = 1;
		}
		mycache[set].putOld(last.first, tempBlock);
	}
   }
   patternBuffQueue.push_front(std::make_pair(key, wc));

}




int main() {
   using namespace simplewcount;
   
   fillMainMemory(FILE_NAME);
   printMainMemory();
   
   int totalPredict = 0;
   int correctPredict = 0;


   std::ifstream infile(FILE_NAME);
   std::string fileLine;

   while (std::getline(infile, fileLine)) {

        std::string instrAddr, memOp, memAddr;
        std::istringstream traces(fileLine);
        traces >> instrAddr >> memOp >> memAddr;

        //TODO get set bits
        unsigned long cacheLine;
        cacheLine = std::stoul(memAddr, nullptr, 16) / 64;
        //Create a mask(parameter) AND it with cacheline
        int cacheSet = 0;

        //Check if exists in cache
        if(!mycache[cacheSet].exists(cacheLine)) {
	        totalPredict++;

                int actualWc = getActualwc(cacheLine);
                //Check that pwc buffer is not empty
                if (!pwcBuffQueue.empty()) {
		
			std::cout << "pwc buffer not empty" << std::endl;
			//Compare predicted wc and actual wc
                        int predictStatus = 0;
                        /* Read the PWCbuffer, check if predition matches actual
                         * If yes update coverage
                         * Either case flush the buffer*/
                        /* TODO check how many times prediction was correct
                         * Can be used to tell longer history sizes work*/
                        for (int i = 0; i <= PWCBUFFER_SIZE; ++i) {
                                if ((actualWc ==  pwcBuffQueue.front()) && (!predictStatus)) {
					std::cout << "A: " << actualWc << " P: " << pwcBuffQueue.front() << std::endl;
                                        predictStatus = 1;
                                }
                                pwcBuffQueue.pop();
                        }
			//Update coverage
                        correctPredict = correctPredict + predictStatus;
                }

		struct Cacheblock tempBlock = getBlock(cacheLine);
                /*Check this: Doing twice*/
		if (memOp == "W") {
			tempBlock.isDirty = 1;
		}	
		//Put block in cache	
		putinCache(cacheSet, cacheLine, tempBlock);
		//Put pwc buffer
		for (int i = 0; i < WCHISTORY_SIZE; ++i) {
			for (int j = 0; j <= PREDICT_RANGE; ++j) {
				pwcBuffQueue.push(tempBlock.wcHistory[i] + j);
			}
		}	
		//TODO pattern buffer logic
		printPattern();
		updatePattern(cacheLine, actualWc);
        }
	//Update dirty status
	updateDirty (memOp, cacheSet, cacheLine);	

	std::cout << "Correct # of predictions: " << correctPredict << std::endl;
   	std::cout << "Total # of predictions: " << totalPredict << std::endl << std::endl;
	printMainMemory();
   }

   return 0;
}
