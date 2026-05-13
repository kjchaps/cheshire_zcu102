#include "s251.h"

void s251(uint32_t LEN,  uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
	TYPE * restrict b = src();
	TYPE * restrict c = src();
	TYPE * restrict d = src();

	volatile TYPE *reg_ptr = a;
	TYPE buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
		buffer[i] = a[i];
	}

	start_counters();

	float s;

	for (int nl = 0; nl < 4*ntimes; nl++) {
		for (int i = 0; i < LEN; i++) {
			s = b[i] + c[i] * d[i];
			buffer[i] = s * s;
		}
		dummy(nl);
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}
	complete_results(__func__);
}




