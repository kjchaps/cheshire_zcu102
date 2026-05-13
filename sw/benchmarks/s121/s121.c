#include "s121.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)


void s121(uint32_t LEN, uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
        TYPE * restrict b = src();

        volatile TYPE *reg_ptr = a;
        TYPE buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
                buffer[i] = a[i];
        }
	
	start_counters();
	 
	int j;
	for (int nl = 0; nl < 3 * ntimes; nl++) {
		for (int i = 0; i < LEN - 1; i++) {
			j = i+1;  
			buffer[i] = buffer[j] + b[i];
		}
		dummy(nl);
	}
    
	stop_counters();

	*SCALAR_RES_ADDR = (uint32_t)j;
	
	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

 	complete_results(__func__);
}
