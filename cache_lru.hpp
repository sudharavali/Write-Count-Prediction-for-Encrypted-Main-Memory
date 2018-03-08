#ifndef CACHE_LRU_HPP
#define	CACHE_LRU_HPP

#include <unordered_map>
#include <list>
#include <stdexcept>
#include <queue>

namespace simplewcount {

const int WCHISTORY_SIZE = 2;

struct Cacheblock {
	unsigned int data;
	int wcActual;
	int wcHistory[WCHISTORY_SIZE];
	int dirtyStatus;
};

class LruCache {
	public:
		typedef typename std::pair<unsigned long, struct Cacheblock> blockPair;
		typedef typename std::list<blockPair>::iterator listIterator;

		LruCache(unsigned int maxSize) :
			cachesize(maxSize) {
		}
		
		unsigned int size() const {
			return cacheMap.size();
		}

		void put(const unsigned long& key, struct Cacheblock& value) {
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
		
		bool exists(const unsigned long& key) const {
			return cacheMap.find(key) != cacheMap.end();
		}

		const struct Cacheblock& get(const unsigned long& key) {
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
		std::unordered_map<unsigned long, listIterator> cacheMap;
		unsigned int cachesize;
};


// Main memory (mainMemory)
std::unordered_map<unsigned long, int> mainMemory;

// Predicted write count buffer (pwcBuff)
std::queue<unsigned long> pwcBuffQueue; 
std::unordered_map<unsigned long, unsigned int> pwcBuffMap;

void pushNew(const unsigned long& key, const unsigned int& value);
void popOld(const unsigned long& key);


// Pattern buffer (patterBuff)
std::queue<unsigned long> patternBuffQueue; 

} 

#endif	/* CACHE_LRU_HPP */

