#include "s311.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)

void s311(uint32_t LEN,  uint32_t ntimes){
	send_setup_info(LEN, __func__);

	 TYPE * restrict a = src();
	 TYPE sum;

 	 start_counters();
	 
	 for (uint32_t nl = 0; nl < ntimes * 10; nl++) {
		 sum = (float)0.;
		 for (uint32_t i = 0; i < LEN; i++) {
			 sum += a[i];
		 }
		 dummy(nl); 
	 }
	 
	 stop_counters();
	 
	 *SCALAR_RES_ADDR = sum; fence();
	 
	 complete_results(__func__);
}
