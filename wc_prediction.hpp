#ifndef WC_PREDICTION_HPP
#define	WC_PREDICTION_HPP

#include <unordered_map>
#include <list>
#include <stdexcept>
#include <queue>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "parameter.hpp"


namespace wcprediction {

typedef unsigned long addr_t;
// Total number of cache lines in cache
const unsigned int CACHE_SIZE = PARAM_CACHE_SIZE;
// Number of cache lines stored in a set
const int SET_SIZE = PARAM_SET_SIZE;
// Number of sets
const int SET_NUM = CACHE_SIZE / SET_SIZE;
// Size of set bits
const int SET_BITS = floor(log(SET_NUM)/log(2));
// Size of write count history
const int WCHISTORY_SIZE = PARAM_WCHISTORY_SIZE;
// Size of pattern FIFO
const int PATTERNFIFO_SIZE = WCHISTORY_SIZE + 1;
// Range of write count predictions
const int PREDICT_RANGE = PARAM_PREDICT_RANGE;
// Frequency of dirty writebacks due to wc history holders
const int WCHISTORY_WRITEBACK_FREQUENCY = 1;

// Address traces file
const std::string FILE_NAME = "addrtrace/trace.txt";

// Memory block
struct CacheBlock {

	// int some_data
	int wc_actual;
	int wc_history[WCHISTORY_SIZE];
	int is_dirty;
};



// Cache with LRU replacement policy class
class LruCache {
	public:
		typedef typename std::pair<addr_t, struct CacheBlock> BlockPair;
		typedef typename std::list<BlockPair>::iterator ListIterator;

		LruCache() :
			cache_size_(SET_SIZE) {
		}
		
		unsigned int Size() const {
			return cache_map_.size();
		}
 		// Get the address of LRU block
		addr_t GetLeastUsed() {

			auto last = cache_list_.end();
			last--;
			return last->first;
		}
		// Put block in $ and update it based on LRU
		void PutAndUpdate(const addr_t& key, const struct CacheBlock& value) {
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
		void PutNotUpdate(const addr_t& key, const struct CacheBlock& value) {
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
		bool Exists(const addr_t& key) const {
			return cache_map_.find(key) != cache_map_.end();
		}
		// Get block from $ and update it based on LRU
		const struct CacheBlock& GetAndUpdate(const addr_t& key) {
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
		const struct CacheBlock& GetNotUpdate(const addr_t& key) {
			auto it = cache_map_.find(key);
			if (it == cache_map_.end()) {
				throw std::range_error("NO SUCH KEY");
			} 
			else {
				return it->second->second;
			}
		}	
	private:
		std::list<BlockPair> cache_list_;
		std::unordered_map<addr_t, ListIterator> cache_map_;
		unsigned int cache_size_;
};


// Initialize set mask
void InitMask(); 
int CompareWriteCounts(const addr_t& key);

//Main memory (MM)
std::unordered_map<addr_t, CacheBlock> main_memory;

int GetActualWcFromMM(const addr_t& key);
struct CacheBlock GetBlockFromMM(const addr_t& key);

//Cache($)
LruCache lru_cache[SET_NUM];

void PutBlockInCache(const int& set, const addr_t& key, const struct CacheBlock& block);
void UpdateDirtyStatus(const std::string& op, const int& set, const addr_t& key);

//Predicted write count (pwc) buffer
std::queue<int> pwc_buffer; 


//Pattern buffer
std::list<std::pair<addr_t, int>> pattern_buffer; 

void UpdatePattern(const addr_t& key, const int& wc);

extern addr_t mask_set;
extern int total_predictions;
extern int correct_prediction;
extern unsigned int total_bandwidth;
extern unsigned int write_bandwidth;
extern unsigned int prediction_bandwidth;
} 

#endif	/* WC_PREDICTION_HPP */

