#include "wc_prediction.hpp"

addr_t mask_set;
int total_predictions;
int correct_predictions;
int misses;
unsigned int total_bandwidth;
unsigned int write_bandwidth;
unsigned int prediction_bandwidth;

unsigned int LruCache::Size() const {
   return cache_map_.size();
}

// Get the address of LRU block

addr_t LruCache::GetLeastUsed() {

   auto last = cache_list_.end();
   last--;
   return last->first;

}

// Put block in $ and update it based on LRU
void LruCache::PutAndUpdate(const addr_t& key, const struct CacheBlock& value) {
   auto it = cache_map_.find(key);
   cache_list_.push_front(BlockPair(key, value));
   if (it != cache_map_.end()) {
	cache_list_.erase(it->second);
	cache_map_.erase(it);
   }
   cache_map_[key] = cache_list_.begin();

   if (cache_map_.size() > cache_size_) {
	auto last = cache_list_.end();
	last--;
	cache_map_.erase(last->first);
	cache_list_.pop_back();
   }


}

// Put block in $ without updating
void LruCache::PutNotUpdate(const addr_t& key, const struct CacheBlock& value) {
   auto it = cache_map_.find(key);
   if (it != cache_map_.end()) {
	cache_map_.erase(it);
	auto old = cache_list_.erase(it->second);

	old = cache_list_.insert(old,BlockPair(key,value));
	cache_map_[key] = old;
   }

   if (cache_map_.size() > cache_size_) {
	throw std::range_error("CACHE OVERFLOW");
   }
}

// Check if block is in $
bool LruCache::Exists(const addr_t& key) const {
   return cache_map_.find(key) != cache_map_.end();
}

// Get block from $ and update it based on LRU
const struct CacheBlock& LruCache::GetAndUpdate(const addr_t& key) {
   auto it = cache_map_.find(key);
   if (it == cache_map_.end()) {
	throw std::range_error("NO SUCH KEY");
   }
   else {
	cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
	return it->second->second;
   }
}

// Get block from $ without updating
const struct CacheBlock& LruCache::GetNotUpdate(const addr_t& key) {
   auto it = cache_map_.find(key);
   if (it == cache_map_.end()) {
	throw std::range_error("NO SUCH KEY");
   }
   else {
	return it->second->second;
   }
}

/*
int CompareWriteCounts(const addr_t& key){

   total_predictions++;

   const int actual_wc = GetActualWcFromMM(key);
   // Check if predicted write count (pwc) buffer is not empty
   // If yes, skip
   if (!pwc_buffer.empty()) {

	// Compare predicted wc and actual wc
	int predict_status = 0;
	// Read the pwc buffer, check if predition matches actual
	// If yes update coverage
	// Either case flush the buffer
	while(!pwc_buffer.empty()) {
		if ((actual_wc ==  pwc_buffer.front()) && (!predict_status)) {
			predict_status = 1;
		}
		pwc_buffer.pop();
	}
	// Update coverage
	correct_predictions = correct_predictions + predict_status;
   }

   return actual_wc;
}
*/


void InitMask(){

   total_predictions = 0;
   correct_predictions = 0;
   misses =0;

   total_bandwidth = 0;
   write_bandwidth = 0;
   prediction_bandwidth = 0;

   mask_set = 0x0;
   for (int i = 0; i < SET_BITS; i++) {
	mask_set = mask_set | (0x1 << i );
   }
}



void PutBlockInCache(const int& set, const addr_t& key, const struct CacheBlock& block) {
   // Check if $ is full:
   // If yes find LRU block to evict (E) to put new block (A) in $
   if (lru_cache[set].Size() >= SET_SIZE) {
	addr_t evict_key = lru_cache[set].GetLeastUsed();
	struct CacheBlock evict_block = lru_cache[set].GetNotUpdate(evict_key);
	// Check E dirty status, if yes update wc and writeback to MM
	if (evict_block.is_dirty) {
		if (evict_block.is_dirty == 1) {
			write_bandwidth++;
		}
		else if (evict_block.is_dirty == 2) {
			prediction_bandwidth++;
		}

		evict_block.is_dirty = 0;
		//evict_block.wc_actual++;
		// Writeback E to MM
		main_memory[evict_key] = evict_block;

		// Increment total writeback bandwidth
		total_bandwidth++;
	}
   }
   // $ is not full, put A in $
   lru_cache[set].PutAndUpdate(key, block);

}

void UpdateDirtyStatus(const std::string& op, const int& set, const addr_t& key) {
   // Check the memory operation, if WRITE then block is dirty
   struct CacheBlock temp_block = lru_cache[set].GetAndUpdate(key);
   if (op == "W") {
   	temp_block.is_dirty = 1;
   }
   // Even in case of READ we need to update $
   lru_cache[set].PutAndUpdate(key, temp_block);
}

/*


int GetActualWcFromMM(const addr_t& key) {

   auto it = main_memory.find(key);
   if (it == main_memory.end()) {
	return 0;
   }
   else {
	return it->second.wc_actual;
   }

}

*/

struct CacheBlock GetBlockFromMM(const addr_t& key) {

   struct CacheBlock temp_block = {};
   auto it = main_memory.find(key);
   if (it == main_memory.end()) {
	return temp_block;
   }
   else {
	temp_block = it->second;
	return temp_block;
   }

}


void UpdatePattern(const addr_t& key) {
   // Check if pattern buffer is full, if not skip
   if (pattern_buffer.size() == PATTERNFIFO_SIZE) {
	// Get last block from the buffer
	// Fill it with predicted write counts (history holder)
	addr_t last = pattern_buffer.back();
	pattern_buffer.pop_back();

	addr_t set = last & mask_set;

	if (lru_cache[set].Exists(last)) {
		struct CacheBlock temp_block = lru_cache[set].GetNotUpdate(last);
		int i = 0;
		for (auto it = pattern_buffer.crbegin(); it != pattern_buffer.crend(); ++it) {
			temp_block.wc_history[i] = *it; /* second ?? */
                        i++;
		}
		if (!temp_block.is_dirty) {
			//if (total_predictions % WCHISTORY_WRITEBACK_FREQUENCY == 0) {
				temp_block.is_dirty = 2;
			}

		lru_cache[set].PutNotUpdate(last, temp_block);
	}
   }
   pattern_buffer.push_front(key);

}




int main() {

   InitMask();

   std::ifstream infile(FILE_NAME);
   std::string fileLine;

   while (std::getline(infile, fileLine)) {

        std::string mem_operation, byte_address;
        std::istringstream traces(fileLine);
        traces >> mem_operation >> byte_address;

        unsigned long cache_line;
        cache_line = std::stoul(byte_address, nullptr, 16) / 64;
        int cache_set;
        int var;
        //int set;
        //auto last = cache_list_.end();
      //  std::pair<addr_t, int> last = pattern_buffer.back();


       // addr_t set = last.first & mask_set;


       // int correct_predictions ;
        int wc_history[WCHISTORY_SIZE];
	    cache_set = cache_line & mask_set;

        unsigned long prefetch_line;
       // prefetch_line = std::stoul(byte_address, nullptr, 16) / 64;
        int prefetch_set;
        prefetch_set = prefetch_line & prefetch_set;

        //unsigned long prefetched_line;
        //prefetched_line = std::stoul(byte_address, nullptr, 16) / 64;
        //int prefetched_set;
        //prefetched_set = prefetched_line & prefetched_set;






        // Check if new block (A) exists in $, if yes do nothing
        if(!lru_cache[cache_set].Exists(cache_line)) {

                //its a MISS :

		 struct CacheBlock temp_block = GetBlockFromMM(cache_line);
            PutBlockInCache(cache_set, cache_line, temp_block);


            struct CacheBlock prefetch_block = {};
            for(int i = 0; i < 2; i++ ) {
                    addr_t prefetch_line = temp_block.wc_history[i];
                    prefetch_set = prefetch_line & mask_set;
                    prefetch_block = GetBlockFromMM(prefetch_line);
                    prefetch_block.var = 1;

                    PutBlockInCache(prefetch_set, prefetch_line, prefetch_block);
            }
              total_predictions = total_predictions +2 ;
              misses = misses +1;
            //    total_predictions++;



            std::cout<< " cache size"<< lru_cache[cache_set].Size() << std::endl;
            std::cout<< " prefetch cache size"<< lru_cache[prefetch_set].Size() << std::endl;

        }


        else
            {
                // its a hit
                std::cout<< " its a hit" << std ::endl;


                struct CacheBlock temp_block = lru_cache[cache_set].GetNotUpdate(cache_line);


                    if (temp_block.var == 1){
                        std::cout<< " its checking " << std ::endl;
                        temp_block.var = 0;

                      correct_predictions = correct_predictions + 1;
                     std::cout<< " its checking after "<< correct_predictions<< std ::endl;


                    }
                    PutBlockInCache(cache_set, cache_line, temp_block);

            }
                UpdatePattern(cache_line);
                UpdateDirtyStatus (mem_operation, cache_set, cache_line);
            }




            //UpdateDirtyStatus (mem_operation, cache_set, cache_line);

            // PutBlockInCache(cache_set, cache_line, temp_block);

                 // struct CacheBlock prefetched_block = {};
                //for(int i = 0; i < WCHISTORY_SIZE; i++ ) {
                  //  addr_t prefetched_line = temp_block.wc_history[i];
                    //prefetched_set = prefetched_line & mask_set;
                    //prefetched_block = lru_cache[0].GetNotUpdate(prefetched_line);



























/*


		int actual_wc;
		// Compare actual wc with predicts wc from pwc buffers
		actual_wc = CompareWriteCounts(cache_line);

		struct CacheBlock temp_block = GetBlockFromMM(cache_line);

		// Put block in cache
		PutBlockInCache(cache_set, cache_line, temp_block);

		// Put new predicted write counts in PWCbuffer
		for (int i = 0; i < WCHISTORY_SIZE; ++i) {
			for (int j = 0; j <= PREDICT_RANGE; ++j) {
				pwc_buffer.push(temp_block.wc_history[i] + j);
			}
		}

		*/






		// Put block in pattern buffer
		//UpdatePattern(cache_line);
        //}
	// Update dirty status
	//UpdateDirtyStatus (mem_operation, cache_set, cache_line);
   //}

   std::cout << "PASS" << std::endl;
   std::cout << "Summary: ";
   std::cout << " Total Misses =" << misses;
   std::cout << "CorrectPrediction=" << correct_predictions;
   std::cout << " TotalPrediction=" << total_predictions << std::endl;


#ifdef DEBUG
   std::cout << "Bandwidth: ";
   std::cout << "PredictionBandwidth=" << prediction_bandwidth;
   std::cout << " WriteBandwidth=" << write_bandwidth;
   std::cout << " TotalBandwidth=" << total_bandwidth << std::endl;
#endif

   return 0;
}
//W 0x3ecdb612
//W 0xdc232573
//W 0x50e491b4
// It doesnt take into account if it comes back another time. Thats how prof said the algorithm is supposed to work.
