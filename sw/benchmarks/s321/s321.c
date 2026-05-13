#include "s321.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)


void s321(uint32_t LEN, uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
	TYPE * restrict b = src();

	volatile TYPE *reg_ptr = a;
	TYPE buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
		buffer[i] = a[i];
	}

	start_counters();


	for (int nl = 0; nl < ntimes; nl++) {
		for (int i = 1; i < LEN; i++) {
			buffer[i] += buffer[i-1] * b[i]; 
		}
		dummy(nl);
	}

	stop_counters();

//	*SCALAR_RES_ADDR = (uint32_t)j;

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

	complete_results(__func__);
}

