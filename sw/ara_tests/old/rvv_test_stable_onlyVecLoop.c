#include <stdint.h>
#include <string.h>
#include "util.h"

#define DONE_ADDR   ((volatile uint32_t *)0x9EEC0020ULL)
#define OUT_ADDR    ((volatile uint8_t  *)0x9EED0000ULL)
#define C_LAST_ADDR ((volatile uint32_t *)0x9EEC0040ULL)
#define TEMP_ADDR ((volatile uint32_t *)0x9EEC0030ULL)




typedef struct __attribute__((packed)) {
  char test_name[8];
  char metric_name[8];
  uint32_t value_lo;
  uint32_t value_hi;
} benchmark_result_t;

static inline void fence_rw_rw(void) {
  __asm__ volatile("fence" ::: "memory");
}

uint64_t vectorized_loop(uint64_t* A, uint64_t* B, uint64_t* C, uint64_t size)
{
    uint64_t r = 0;
    for (uint64_t k = 0; k < size; k++)
        C[k] += A[k] + B[k];
    for (uint64_t k = 0; k < size; k++)
        r += C[k];
    return r;
}


int main(void) {
	  *TEMP_ADDR = 0x2255;
  volatile benchmark_result_t *out = (volatile benchmark_result_t *)OUT_ADDR;
////////////////////////////////////////////////////
  
   enum { SIZE = 100 };

    uint64_t A[SIZE];
    uint64_t B[SIZE];
    uint64_t C[SIZE];

    for (uint64_t i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = 17 + i;
        C[i] = 0;
    }
  uint64_t start = get_mcycle();

    uint64_t r = vectorized_loop(A, B, C, SIZE);
  
  
  uint64_t end = get_mcycle();
  ////////////////////////////////////////////////////
  fence_rw_rw();
*TEMP_ADDR = 0xCCDD;
  *C_LAST_ADDR = r;
  fence_rw_rw();

  uint64_t delta = end - start;

  memset((void *)out, 0, sizeof(benchmark_result_t));
  memcpy((void *)out->test_name, "vectest", 8);
  memcpy((void *)out->metric_name, "mcycles", 8);
  out->value_lo = (uint32_t)(delta & 0xffffffffu);
  out->value_hi = (uint32_t)(delta >> 32);

  fence_rw_rw();
  *DONE_ADDR = 0xf005ba11;
  fence_rw_rw();
  fence_rw_rw();
  fence_rw_rw();

  while (1) { }
  return 0;
}
