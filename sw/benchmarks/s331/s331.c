#include "s331.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)

void s331(uint32_t LEN, uint32_t ntimes){

	send_setup_info(LEN, __func__);
	TYPE * restrict a = src();
	TYPE sum;

	start_counters();

	int j;
	float chksum;
	for (int nl = 0; nl < ntimes; nl++) {
		j = -1;
		for (int i = 0; i < LEN; i++) {
			if (a[i] < (float)0.) {
				j = i;
			}
		}
		chksum = (float)j;
		dummy(nl);
	}

	stop_counters();

	*SCALAR_RES_ADDR = (int32_t)chksum; fence(); 

	complete_results(__func__);
}




