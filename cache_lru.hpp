#ifndef CACHE_LRU_HPP
#define	CACHE_LRU_HPP

#include <unordered_map>
#include <list>
#include <stdexcept>
#include <queue>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


namespace simplewcount {

typedef unsigned long addr_t;

unsigned int CACHE_SIZE = 65535;
const int SET_NUM = 2;
const unsigned int SET_SIZE = CACHE_SIZE / SET_NUM;

const int WCHISTORY_SIZE = 2;
const int PWCBUFFER_SIZE = 1000; 
const int PATTERNFIFO_SIZE = WCHISTORY_SIZE + 1;

const std::string FILE_NAME = "addr_traces";

struct Cacheblock {
	unsigned int data;
	int wcActual;
	int wcHistory[WCHISTORY_SIZE];
	int dirtyStatus;
};

class LruCache {
	public:
		typedef typename std::pair<addr_t, struct Cacheblock> blockPair;
		typedef typename std::list<blockPair>::iterator listIterator;

		LruCache(unsigned int maxSize) :
			cachesize(maxSize) {
		}
		
		unsigned int size() const {
			return cacheMap.size();
		}

		void put(const addr_t& key, struct Cacheblock& value) {
			auto it = cacheMap.find(key);
			cacheList.push_front(blockPair(key, value));
			if (it != cacheMap.end()) {
				cacheList.erase(it->second);
				cacheMap.erase(it);
			}
			cacheMap[key] = cacheList.begin();
			
			if (cacheMap.size() > cachesize) {
				auto last = cacheList.end();
				last--;
				cacheMap.erase(last->first);
				cacheList.pop_back();
			}
		}
		
		bool exists(const addr_t& key) const {
			return cacheMap.find(key) != cacheMap.end();
		}

		const struct Cacheblock& get(const addr_t& key) {
			auto it = cacheMap.find(key);
			if (it == cacheMap.end()) {
				throw std::range_error("NO SUCH KEY");
			} else {
				cacheList.splice(cacheList.begin(), cacheList, it->second);
				return it->second->second;
			}
		}	
	private:
		std::list<blockPair> cacheList;
		std::unordered_map<addr_t, listIterator> cacheMap;
		unsigned int cachesize;
};


//Main memory (mainMemory)
std::unordered_map<addr_t, Cacheblock> mainMemory;

void fillMainMemory(std::string filename);
int getActualwc(const addr_t& key);

//Cache($)
LruCache mycache[SET_NUM](SET_SIZE);

void testCache();

//Predicted write count buffer (pwcBuff)
std::queue<addr_t> pwcBuffQueue; 
std::unordered_map<addr_t, int> pwcBuffMap;

void pushNew(const addr_t& key, const int& value);
void popOld();

//Pattern buffer (patterBuff)
std::list<addr_t> patternBuffQueue; 

} 

#endif	/* CACHE_LRU_HPP */

