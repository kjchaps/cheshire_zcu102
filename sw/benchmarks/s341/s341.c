#include "s341.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)

void s341(uint32_t LEN, uint32_t ntimes){

	send_setup_info(LEN, __func__);
	TYPE * restrict a = src();
	TYPE * restrict b = src();

	volatile TYPE *reg_ptr = a;
	TYPE buffer[LEN];

	start_counters();

	int j;
	for (int nl = 0; nl < ntimes; nl++) {
		j = -1;
		for (int i = 0; i < LEN; i++) {
			if (b[i] > (float)0.) {
				j++;
				buffer[j] = b[i];
			}
		}
		dummy(nl);
	}

	stop_counters();


	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

	*SCALAR_RES_ADDR = 0xAABB; 
	complete_results(__func__);
}




