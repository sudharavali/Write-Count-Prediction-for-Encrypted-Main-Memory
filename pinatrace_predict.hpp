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



typedef unsigned long addr_t;
// Total number of cache lines in cache
const unsigned int CACHE_SIZE = PARAM_CACHE_SIZE;
// Number of cache lines stored in a set
const int SET_SIZE = PARAM_SET_SIZE;
// Number of sets
const int SET_NUM = CACHE_SIZE / SET_SIZE;
// Size of set bits
const int SET_BITS = floor(log((double)SET_NUM)/log(2.0));
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
		
		unsigned int Size() const;

		// Get the address of LRU block
		addr_t GetLeastUsed();

		// Put block in $ and update it based on LRU
		void PutAndUpdate(const addr_t& key, const struct CacheBlock& value);

		// Put block in $ without updating
		void PutNotUpdate(const addr_t& key, const struct CacheBlock& value);
		
		// Check if block is in $
		bool Exists(const addr_t& key) const;

		// Get block from $ and update it based on LRU
		const struct CacheBlock& GetAndUpdate(const addr_t& key);

		// Get block from $ without updating
		const struct CacheBlock& GetNotUpdate(const addr_t& key);

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
extern unsigned long long total_predictions;
extern unsigned long long correct_predictions;

#endif	/* WC_PREDICTION_HPP */

