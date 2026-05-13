#pragma once

#include <stdint.h>
#include "regs/cheshire.h"
#include "params.h"
#include <riscv_vector.h>
#include "encoding.h"
#include <string.h>
#include "types.h"

#define DONE_ADDR   		((volatile uint32_t *)0x9EEC0020ULL)
#define START_ADDR        	((volatile uint32_t *)0x9EEC0030ULL)
#define INIT_ADDR         	((volatile uint32_t *)0x9EEC0040ULL)
#define ARR_ADDR 		((volatile uint32_t *)0x9EEC0050ULL)
#define TEMP_ADDR               ((volatile char     *)0x9EEC0060ULL)
#define NAME_ADDR         	((volatile char     *)0x9EEC0100ULL)
#define EW_ADDR           	((volatile char	    *)0x9EEC0110ULL)
#define LEN_ADDR          	((volatile uint32_t *)0x9EEC0120ULL)
#define RESULTS_ADDR    	((volatile uint8_t  *)0x9EED0000ULL)
#define ARR_BASE_PTR_ADDR 	((volatile uint32_t *)0x9EEE0000ULL)

#define SCALAR_RES_ADDRA ((volatile uint32_t *)0x9FFF0180ULL)
#define SCALAR_RES_ADDRB ((volatile uint32_t *)0x9FFF0000ULL)

#define ARRAY_BUFFER_STRIDE  0x1000U
static inline uintptr_t PREV_ARR_END_ADDR; //= (uintptr_t)ARR_BASE_PTR_ADDR;
static inline char *EW_stored;
static inline uint32_t LEN_stored;
static inline char *name_stored;

//unsigned int timer;
static inline int64_t timer;
static inline int64_t instret; 
static inline uint64_t delta_minst;
static inline uint64_t delta_mcycle;

typedef struct __attribute__((packed)) {
	char test_name[8];
	uint32_t minst_lo;
	uint32_t minst_hi;
	uint32_t mcycle_lo;
	uint32_t mcycle_hi;
} benchmark_result_t;


static inline void dummy(int a){
	// --  called in each loop to make all computations appear required

	*TEMP_ADDR = a; fence(); 
}

static inline void wait_for_trigger(volatile uint32_t *addr){
	while (*addr != 0xCAFEBABE) {
		fence();
	}
	*addr = 0;
	fence();
}

static inline void reset_array_allocator(void) {
	PREV_ARR_END_ADDR = (uintptr_t)ARR_BASE_PTR_ADDR;
	EW_stored = "";
	LEN_stored = 0;
	name_stored = "";

}

static inline void send_setup_info(uint32_t LEN, const char *test_name){
	reset_array_allocator(); 
	wait_for_trigger(START_ADDR); 
	EW_stored = TYPE_NAME; 
	LEN_stored = LEN; 
	name_stored = test_name;
	strcpy((char *)NAME_ADDR, test_name);
	strcpy((char *)EW_ADDR, TYPE_NAME);
	*LEN_ADDR = LEN;
	*INIT_ADDR = 0xBABEBABE;
	fence();
	wait_for_trigger(ARR_ADDR);
	//could have this receive info on num of arrays and return it 
}

static inline void setup_fs_vs(){
	asm volatile ("li t0, %0" :: "i"(MSTATUS_FS));
	asm volatile ("csrs mstatus, t0");
	asm volatile ("li t0, %0" :: "i"(MSTATUS_VS));
	asm volatile ("csrs mstatus, t0");
	fence();
}

static inline void sendDone(void){
	fence();
	*DONE_ADDR = 0xf005ba11;
	fence();
}

static inline void finalTestComplete(void){
	fence(); 
	*DONE_ADDR = 0xDECAFBAD; 
	fence();
}

// Return the current value of the cycle counter
static inline int64_t get_cycle_count() {
	int64_t cycle_count;
	// The fence is needed to be sure that Ara is idle, and it is not performing
	// the last vector stores when we read mcycle with stop_timer()
	asm volatile("fence; csrr %[cycle_count], cycle" : [cycle_count] "=r"(cycle_count));
	return cycle_count;
};

// Start and stop the counter
static inline void start_timer() { timer = -get_cycle_count(); }
static inline void stop_timer() { timer += get_cycle_count(); }

// Get the value of the timer
static inline int64_t get_timer() { return timer; }

// Return the current value of the instret
static inline int64_t get_instret_count() {
	int64_t instret_count;
	asm volatile("fence; csrr %[instret_count], instret" : [instret_count] "=r"(instret_count));
	return instret_count;
};

static inline void start_instret() { instret = -get_instret_count(); }
static inline void stop_instret() { instret += get_instret_count(); }

static inline int64_t get_instret() { return instret; }

static inline void stop_counters(){
	stop_timer(); 
	stop_instret(); 
	//delta_minst = get_instret_count();
	////delta_mcycle = get_cycle_count();
}

static inline void start_counters(){
	start_instret();
	start_timer();
}

static inline void writeResults(const char *name, uint64_t delta_minst, uint64_t delta_mcycle){
	//name = name_stored; 
	volatile benchmark_result_t *out = (volatile benchmark_result_t *)RESULTS_ADDR;
	memset((void *)out, 0, sizeof(benchmark_result_t));
	if (name != NULL) {
		strncpy((char *)out->test_name, name, sizeof(out->test_name) - 1);
	}

	out->minst_lo  = (uint32_t)(delta_minst & 0xffffffffu);
	out->minst_hi  = (uint32_t)(delta_minst >> 32);
	out->mcycle_lo = (uint32_t)(delta_mcycle & 0xffffffffu);
	out->mcycle_hi = (uint32_t)(delta_mcycle >> 32);
}

static inline void complete_results(const char *test_name){
	delta_mcycle = get_timer();
	delta_minst = get_instret(); 
	writeResults(test_name, delta_minst, delta_mcycle);
	sendDone();
}
/*
   static inline volatile uint32_t *src_uint32(void)
   {
   uintptr_t base_addr = PREV_ARR_END_ADDR;
   PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
   return (volatile uint32_t *)(uintptr_t)base_addr;
   }

   static inline volatile uint8_t *src_uint8(void)
   {
   uintptr_t base_addr = PREV_ARR_END_ADDR;
   PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
   return (volatile uint8_t *)(uintptr_t)base_addr;
   }

   static inline volatile int32_t *src_int32(void)
   {
   uintptr_t base_addr = PREV_ARR_END_ADDR;
   PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
   return (volatile int32_t *)(uintptr_t)base_addr;
   }

   static inline volatile int8_t *src_int8(void)
   {
   uintptr_t base_addr = PREV_ARR_END_ADDR;
   PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
   return (volatile int8_t *)(uintptr_t)base_addr;
   }

   static inline float *src_float(void)
   {
   uintptr_t base_addr = PREV_ARR_END_ADDR;
   PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
   return (volatile float *)(uintptr_t)base_addr;
   }
   */


static inline uint32_t checker =0; 

static inline volatile TYPE *src(void)
{
	uintptr_t ptr = *(volatile uintptr_t *)PREV_ARR_END_ADDR;

	uintptr_t addr = *(volatile uintptr_t *)ptr;
	if (checker < 1){
		*(volatile uintptr_t *)SCALAR_RES_ADDRA = ptr;
		*(volatile uintptr_t *)SCALAR_RES_ADDRB = addr;

		fence();
		checker++; 
	}
	PREV_ARR_END_ADDR += 0x10;

	return (volatile TYPE *)ptr;
}

/*
   static inline TYPE *src(void)
   {
   if(checker < 1){
 *SCALAR_RES_ADDRA = (uint32_t)PREV_ARR_END_ADDR;
 fence(); 

 checker++; 
 }


 uintptr_t base_addr = PREV_ARR_END_ADDR;
 PREV_ARR_END_ADDR += ARRAY_BUFFER_STRIDE;
 return (volatile TYPE *)(uintptr_t)base_addr;
 }
 */
