#include "s171.h"

void s171(uint32_t LEN, uint32_t ntimes, int inc){
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
		for (int i = 0; i < LEN; i++) {
			buffer[i * inc] += b[i];
		}
		dummy(nl);
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

	complete_results(__func__);
}




