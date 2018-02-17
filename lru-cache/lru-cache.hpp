#include <iostream>
#include <list>
using namespace std;

// Size of address in bits
#define ADDRESS_BITS 32

#if ADDRESS_BITS == 64
typedef unsigned long addr_t;
#else
typedef unsigned int addr_t;
#endif


// Size of set bits
#define SET_BITS 10

// Size of Offset bits
#define OFFSET_BITS 4

// Size of cache block
#define BLOCK_SIZE 2^OFFSET_BITS

// Size of tag bits
#define TAG_BITS ADDRESS_BIT - SET_BITS - OFFSET_BITS

// Number of sets
#define MAX_SET 2^SET_BITS

// Number of cache lines stored in a set or Associativity
#define SET_SIZE 4

//Each cache line should store data 
//and tag bits. This can be exteded to store write history
struct cacheData_struct{
        //char dummydata[BLOCK_SIZE];
        addr_t tag;
        //Add history here
};
typedef struct cacheData_struct cacheData;

std::list<cacheData> cache[MAX_SET];

addr_t mask;
