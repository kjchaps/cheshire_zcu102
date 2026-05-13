#include "s162.h"


void s162_uint8(uint8_t * restrict a, const uint8_t * restrict b, const uint8_t * restrict c, uint32_t LEN, uint32_t ntimes, int k){

	volatile uint8_t *reg_ptr = a;
	uint8_t buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
		buffer[i] = a[i];
	}

	start_counters();

	for (int nl = 0; nl < ntimes; nl++) {
		if (k > 0) {
			for (int i = 0; i < LEN - 1; i++) {
				buffer[i] = buffer[i + k] + b[i] * c[i];
			}
			dummy(nl);
		}
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}
}

void s162(uint32_t LEN, uint32_t ntimes, int k){
	send_setup_info(LEN, __func__);

	TYPE * restrict a = src();
	TYPE * restrict b = src();
	TYPE * restrict c = src(); 

	volatile TYPE *reg_ptr = a;
	TYPE buffer[LEN];

	for (uint32_t i = 0; i < LEN; i++) {
		buffer[i] = a[i];
	}

	start_counters();

	for (int nl = 0; nl < ntimes; nl++) {
		if (k > 0) {
			for (int i = 0; i < LEN - 1; i++) {
				buffer[i] = buffer[i + k] + b[i] * c[i];
			}
			dummy(nl);
		}
	}

	stop_counters();

	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i];
	}

	complete_results(__func__);
}




