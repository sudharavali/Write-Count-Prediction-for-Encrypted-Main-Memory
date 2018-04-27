/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include "pin.H"
#include "wc_prediction.hpp"

FILE * trace;


///////////////////////////////////////////////////////////
// OUR INTEGRATED CODE START
///////////////////////////////////////////////////////////

addr_t mask_set;
unsigned long long total_predictions;
unsigned long long correct_predictions;
unsigned long long total_bandwidth;
unsigned long long write_bandwidth;
unsigned long long  prediction_bandwidth;

unsigned long long print_count;
int max_wc;
unsigned long long total_fetches;
unsigned long long total_misses;
unsigned long long total_hits;
unsigned long long total_reads;
unsigned long long total_writes;

void InitMask()
{

   total_predictions = 0;
   correct_predictions = 0;

   total_bandwidth = 0;
   write_bandwidth = 0;

   print_count = 0;
   max_wc = 0;
   total_fetches = 0;
   total_misses = 0;
   total_hits = 0;
   total_reads = 0;
   total_writes = 0;

   mask_set = 0x0;
   for (int i = 0; i < SET_BITS; i++) {
	mask_set = mask_set | (0x1 << i );
   }
}

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
	//throw std::range_error("CACHE OVERFLOW");
	// Should never be here, since we control cache size
	std::cout << "CACHE OVERFLOW" << std::endl;
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
	//throw std::range_error("NO SUCH KEY");
	// Should never be here, since we always check first if block exists in the cache
	std::cout << "NO SUCH KEY" << std::endl;
	return it->second->second;
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
	//throw std::range_error("NO SUCH KEY");
	// Should never be here, since we always check first if block exists in the cache
	std::cout << "NO SUCH KEY" << std::endl;
	return it->second->second;
   } 
   else {
	return it->second->second;
   }
}
	
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
	// Either case flush the buffer*/
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
			evict_block.wc_actual++;
		}
		else if (evict_block.is_dirty == 2) {
			prediction_bandwidth++;
		}
	
		evict_block.is_dirty = 0;
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
	total_writes++;
   	temp_block.is_dirty = 1;
   }else if (op == "R") {
 	total_reads++;
   }
   // Even in case of READ we need to update $
   lru_cache[set].PutAndUpdate(key, temp_block);
}

int GetActualWcFromMM(const addr_t& key) {
   
   auto it = main_memory.find(key);
   if (it == main_memory.end()) {
	return 0;
   }
   else {
	return it->second.wc_actual; 
   }

}

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
	
void UpdatePattern(const addr_t& key, const int& wc) {
   // Check if pattern buffer is full, if not skip
   if (pattern_buffer.size() == PATTERNFIFO_SIZE) {
	// Get last block from the buffer 
	// Fill it with predicted write counts (history holder)
	std::pair<addr_t, int> last = pattern_buffer.back();
	pattern_buffer.pop_back();

	addr_t set = last.first & mask_set;

	if (lru_cache[set].Exists(last.first)) {
		struct CacheBlock temp_block = lru_cache[set].GetNotUpdate(last.first);
		int i = 0;
		for (auto it = pattern_buffer.rbegin(); it != pattern_buffer.rend(); ++it) {	
			temp_block.wc_history[i] = it->second;
                        i++;
		}
		if (!temp_block.is_dirty) {
			if (WCHISTORY_WRITEBACK_FREQUENCY != -1) {
				if (total_reads % WCHISTORY_WRITEBACK_FREQUENCY == 0) {
					temp_block.is_dirty = 2;
				}
			}
		}
		lru_cache[set].PutNotUpdate(last.first, temp_block);
	}
   }
   pattern_buffer.push_front(std::make_pair(key, wc));

}

void PredictionTask(const std::string& mem_op, const addr_t& key, const int& set) {	
	
        // Check if new block (A) exists in $, if yes do nothing
        if(!lru_cache[set].Exists(key)) {
		int actual_wc;
		// Compare actual wc with predicts wc from pwc buffers	
		actual_wc = CompareWriteCounts(key);

		// JUST FOR DEBUG
		if (actual_wc > max_wc) {
			max_wc = actual_wc;
		}
		// JUST FOR DEBUG

		struct CacheBlock temp_block = GetBlockFromMM(key);
	
		// Put block in cache	
		PutBlockInCache(set, key, temp_block);

		// Put new predicted write counts in PWCbuffer
		for (int i = 0; i < WCHISTORY_SIZE; ++i) {
			for (int j = 0; j <= PREDICT_RANGE; ++j) {
				pwc_buffer.push(temp_block.wc_history[i] + j);
			}
		}
		// Put block in pattern buffer
		UpdatePattern(key, actual_wc);
		
		total_misses++;
        }else {
		total_hits++;
	}
	// Update dirty status
	UpdateDirtyStatus (mem_op, set, key);

	total_fetches++;
	print_count++;
	if (print_count >= 1000000000) {
		// JUST FOR DEBUG
		std::cout << main_memory.size() << " " << max_wc << std::endl;
		// JUST FOR DEBUG
		print_count = 0;
	}
}

///////////////////////////////////////////////////////////
// OUR INTEGRATED CODE END
///////////////////////////////////////////////////////////


// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr)
{

    unsigned long block_addr = 0;
    block_addr = (long)addr / 64;
    int cache_set = 0;
    cache_set = block_addr & mask_set;
    
    PredictionTask("R", block_addr, cache_set);

    //fprintf(trace,"R %#013lx\n", block_addr);

}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    
    unsigned long block_addr = 0;
    block_addr = (long)addr / 64;
    int cache_set = 0;
    cache_set = block_addr & mask_set;
    
    PredictionTask("W", block_addr, cache_set);

    //fprintf(trace,"W %#013lx\n", block_addr);

}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
    }

}

VOID Fini(INT32 code, VOID *v)
{
    std::cout << "---DONE---" << std::endl;
    unsigned long average_wc = 0;
    unsigned long mainmem_size = 0;
    mainmem_size = main_memory.size();
    for (auto it : main_memory) {
	average_wc += it.second.wc_actual;
    }
    if (mainmem_size) {
	average_wc = average_wc / mainmem_size;
    }
   
 
    fprintf(trace,"===============PARAMETERS===============\n");
    fprintf(trace,"%-25s%-20u\n", "Cache size:", CACHE_SIZE);
    fprintf(trace,"%-25s%-20d\n", "Set size:", SET_SIZE);
    fprintf(trace,"%-25s%-20d\n", "History size", WCHISTORY_SIZE);
    fprintf(trace,"%-25s%-20d\n\n", "Range size", PREDICT_RANGE);
   
    fprintf(trace,"==================LOGS==================\n");
    fprintf(trace,"%-25s%-20.10e\n", "Total fetches", (double)total_fetches);
    fprintf(trace,"%-25s%-20.10e\n", "Unique addresses:", (double)mainmem_size);
    fprintf(trace,"%-25s%-20d\n", "Max write count:", max_wc);
    fprintf(trace,"%-25s%-20lu\n", "Average write count:", average_wc);
    fprintf(trace,"%-25s%-20.10e\n", "WB due to write op:", (double)write_bandwidth);
    fprintf(trace,"%-25s%-20.10e\n", "WB due to wcHH:", (double)prediction_bandwidth);
    fprintf(trace,"%-25s%-20.10e\n", "Total WB to MM:", (double)total_bandwidth);
    fprintf(trace,"%-25s%-20.10e\n", "Total cache misses:", (double)total_misses);
    fprintf(trace,"%-25s%-20.10e\n", "Total cache hits:", (double)total_hits);
    fprintf(trace,"%-25s%-20.10e\n", "Total reads:", (double)total_reads);
    fprintf(trace,"%-25s%-20.10e\n\n", "Total writes:", (double)total_writes);
    
    fprintf(trace,"=================RESULTS================\n");
    fprintf(trace,"%-25s%-20.10e\n", "Correct predictions:", (double)correct_predictions);
    fprintf(trace,"%-25s%-20.10e\n", "Total predictions:", (double)total_predictions);
    fprintf(trace,"%-25s%-20.2f\n", "Coverage:", (float)correct_predictions/total_predictions*100);
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    
    InitMask();

    trace = fopen("pinatrace.out", "w");

    INS_AddInstrumentFunction(Instruction, 0);   
    PIN_AddFiniFunction(Fini, 0);
    
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
