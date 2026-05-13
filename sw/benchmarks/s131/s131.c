#include "s131.h"

void s131(uint32_t LEN, uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
        TYPE * restrict b = src();

        volatile TYPE *reg_ptr = a;
        TYPE buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
		buffer[i] = a[i];
	}

	start_counters();

	int m = 1;
	for (int nl = 0; nl < 5 * ntimes; nl++) {
		for (int i = 0; i < LEN - 1; i++) {
			buffer[i] = buffer[i + m] + b[i];
		}
		dummy(nl);
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

	complete_results(__func__);
}




