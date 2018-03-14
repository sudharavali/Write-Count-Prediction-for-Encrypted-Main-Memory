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
#include "parameter.hpp"


namespace simplewcount {

typedef unsigned long addr_t;

unsigned int CACHE_SIZE = PARAM_CACHE_SIZE;
const int SET_NUM = PARAM_SET_NUM;
const unsigned int SET_SIZE = CACHE_SIZE / SET_NUM;

const int WCHISTORY_SIZE = PARAM_WCHISTORY_SIZE;
const int PATTERNFIFO_SIZE = WCHISTORY_SIZE + 1;

const int PREDICT_RANGE = PARAM_PREDICT_RANGE;
const int PWCBUFFER_SIZE = WCHISTORY_SIZE * PREDICT_RANGE; 

const std::string FILE_NAME = "addr_traces";

struct Cacheblock {
	unsigned int data;
	int wcActual;
	int wcHistory[WCHISTORY_SIZE];
	int isDirty;
};

class LruCache {
	public:
		typedef typename std::pair<addr_t, struct Cacheblock> blockPair;
		typedef typename std::list<blockPair>::iterator listIterator;

		LruCache() :
			cachesize(CACHE_SIZE) {
		}
		
		unsigned int size() const {
			return cacheMap.size();
		}

		addr_t old() {

			auto last = cacheList.end();
			last--;
			return last->first;
		}

		void put(const addr_t& key, const struct Cacheblock& value) {
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
		
		void putOld(const addr_t& key, const struct Cacheblock& value) {
			auto it = cacheMap.find(key);
			if (it != cacheMap.end()) {
				cacheMap.erase(it);
				auto old = cacheList.erase(it->second);
				
				old = cacheList.insert(old,blockPair(key,value));
				cacheMap[key] = old;
			}
			
			if (cacheMap.size() > cachesize) {
				throw std::range_error("CACHE OVERFLOW");
			}
		}

		
		bool exists(const addr_t& key) const {
			return cacheMap.find(key) != cacheMap.end();
		}

		const struct Cacheblock& get(const addr_t& key) {
			auto it = cacheMap.find(key);
			if (it == cacheMap.end()) {
				throw std::range_error("NO SUCH KEY");
			} 
			else {
				cacheList.splice(cacheList.begin(), cacheList, it->second);
				return it->second->second;
			}
		}	
		const struct Cacheblock& getOld(const addr_t& key) {
			auto it = cacheMap.find(key);
			if (it == cacheMap.end()) {
				throw std::range_error("NO SUCH KEY");
			} 
			else {
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
struct Cacheblock getBlock(const addr_t& key);
void printMainMemory();

//Cache($)
LruCache mycache[SET_NUM];

void putinCache(const int& set, const addr_t& key, const struct Cacheblock& block);
void updateDirty(const std::string& op, const int& set, const addr_t& key);

//Predicted write count buffer (pwcBuff)
std::queue<int> pwcBuffQueue; 
std::unordered_map<addr_t, int> pwcBuffMap;

void pushNew(const addr_t& key, const int& value);
void popOld();

//Pattern buffer (patterBuff)
std::list<std::pair<addr_t, int>> patternBuffQueue; 

void updatePattern(const addr_t& key, const int& wc);
void printPattern();

//Coverage = corretPredict/totalPredict
extern int totalPredict;
extern int correctPredict;

} 

#endif	/* CACHE_LRU_HPP */

