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

addr_t mask_set;
int total_predictions;
int correct_predictions;
unsigned int total_bandwidth;
unsigned int write_bandwidth;
unsigned int prediction_bandwidth;

void InitMask()
{

   total_predictions = 0;
   correct_predictions = 0;

   total_bandwidth = 0;
   write_bandwidth = 0;
   prediction_bandwidth = 0;


   mask_set = 0x0;
   for (int i = 0; i < SET_BITS; i++) {
	mask_set = mask_set | (0x1 << i );
   }
}

unsigned int LruCache::Size() const {
   return cache_map_.size();
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

// Check if block is in $
bool LruCache::Exists(const addr_t& key) const {
   return cache_map_.find(key) != cache_map_.end();
}

// Get block from $ without updating
const struct CacheBlock& LruCache::GetNotUpdate(const addr_t& key) {
   auto it = cache_map_.find(key);
   if (it == cache_map_.end()) {
	//throw std::range_error("NO SUCH KEY");
	//Should never be here, since we always check first if block exists in the cache
	return it->second->second;
   } 
   else {
	return it->second->second;
   }
}	
	

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr)
{
    unsigned long block_addr = 0;
    block_addr = (long)addr / 64;
    int cache_set = 0;
    cache_set = block_addr & mask_set;
    
    struct CacheBlock temp_block = {};
    
    if(!lru_cache[cache_set].Exists(block_addr)) {
    	fprintf(trace,"R %#013lx\n", block_addr);
    	temp_block.is_dirty = 0;
    }else {
	temp_block = lru_cache[cache_set].GetNotUpdate(block_addr);
    }
    
    lru_cache[cache_set].PutAndUpdate(block_addr, temp_block);

    //fprintf(trace,"R %p\n", addr);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    unsigned long block_addr = 0;
    block_addr = (long)addr / 64;
    int cache_set = 0;
    cache_set = block_addr & mask_set;
    
    struct CacheBlock temp_block = {};

    if(!lru_cache[cache_set].Exists(block_addr)) {
    	fprintf(trace,"W %#013lx\n", block_addr);
    }else {
	temp_block = lru_cache[cache_set].GetNotUpdate(block_addr);
	if (!temp_block.is_dirty) {
		fprintf(trace,"W %#013lx\n", block_addr);
	}
    }
    
    temp_block.is_dirty = 1;
    lru_cache[cache_set].PutAndUpdate(block_addr, temp_block);

    //fprintf(trace,"R %p\n", addr);
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
    //fprintf(trace, "#eof\n");
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
