#include "s243.h"

void s243(uint32_t LEN, uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
	TYPE * restrict b = src();
	TYPE * restrict c = src();
	TYPE * restrict d = src();
	TYPE * restrict e = src();


	volatile TYPE *reg_ptr_a = a;
	TYPE buffer_a[LEN];
	volatile TYPE *reg_ptr_b = b;
	TYPE buffer_b[LEN];


	for (uint32_t i = 0; i < LEN; i++) {
		buffer_a[i] = a[i];
		buffer_b[i] = b[i];
	}

	start_counters();
	float x = 1.0; 
	for (int nl = 0; nl < ntimes; nl++) {
		for (uint32_t i = 0; i < LEN - 1; i++) {
			buffer_a[i] = c[i]; //buffer_b[i] + c[i] * d[i];
			//buffer_b[i] = buffer_a[i] + d[i] * e[i];
			//buffer_a[i] = buffer_b[i] + buffer_a[i + 1] * d[i];
		}
		dummy(nl);
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr_a[i] = buffer_a[i];
		reg_ptr_b[i] = buffer_b[i];
	}


	complete_results(__func__);
}





