#include "lru-cache.hpp"
void init()
{
        mask = 0x0;
        for(int i = 0; i < SET_BITS+OFFSET_BITS; i++){
                mask = mask | (0x1 << i);
        }
}
list<cacheData>::iterator searchForTag(list<cacheData> &cacheList, addr_t tag)
{
        list<cacheData>::iterator it;
        for(it = cacheList.begin(); it != cacheList.end(); ++it){
                if(it->tag == tag)
                        return it;
        }
        return it;

}

void printCacheList(addr_t set)
{
        list<cacheData> cacheList = cache[set];
        list<cacheData>:: iterator it;
        cout << "set:" << set;
        for(it = cacheList.begin(); it != cacheList.end(); ++it){
                cout << " " << it->tag;
        }
        cout << endl;
}
void put(addr_t address, char data[])
{
        //Get the tag, set bits
        addr_t tag = address >> (SET_BITS + OFFSET_BITS);

        addr_t set = (address & mask)  >> (OFFSET_BITS);

        //Create cache block. TODO copy data
        cacheData cd;
        cd.tag = tag;

        //Search the cache for the tag
        list <cacheData>::iterator it = searchForTag(cache[set], tag);
        
        //The cache block does not exist in the list
        if(it == cache[set].end()){
                //List is full. Need to evict the LRU block
                if(cache[set].size() == SET_SIZE)
                        cache[set].pop_front();
        }else{
                //Cache block exists, remove it, and update block will be put
                //at the end
                cache[set].erase(it);
        }
        cache[set].push_back(cd);
        printCacheList(set);
        return;
}

bool get(addr_t address, cacheData &cd)
{
        //Get the tag, set bits
        addr_t tag = address >> (SET_BITS + OFFSET_BITS);

        addr_t set = (address & mask)  >> (OFFSET_BITS);
        
        //Search the cache for the tag
        list <cacheData>::iterator it = searchForTag(cache[set], tag);

        if(it == cache[set].end()){
                //Cache Miss
                return false;
        }else{
                cd = *it;
                cache[set].erase(it);
                cache[set].push_back(cd);
                printCacheList(set);
                return true;
        }
}

int main()
{
        init();
        cacheData cd;
        
        put(0, NULL);
        put(1<<(SET_BITS+OFFSET_BITS), NULL);
        put(2<<(SET_BITS+OFFSET_BITS), NULL);
        put(3<<(SET_BITS+OFFSET_BITS), NULL);
        get(0,cd);
        put(4<<(SET_BITS+OFFSET_BITS), NULL);
        put(5<<(SET_BITS+OFFSET_BITS), NULL);




        return 0;
}
