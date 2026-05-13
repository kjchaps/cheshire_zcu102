#include "s1232.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)

void s1232(uint32_t LEN,  uint32_t ntimes){
	send_setup_info(LEN, __func__);

	TYPE (*restrict a)[LEN] = (TYPE (*)[LEN])src();
	TYPE (*restrict b)[LEN] = (TYPE (*)[LEN])src();
	TYPE (*restrict c)[LEN] = (TYPE (*)[LEN])src();

	volatile TYPE (*reg_ptr)[LEN] = (volatile TYPE (*)[LEN])a;
	TYPE buffer[LEN][LEN];

	start_counters();

	for (int nl = 0; nl < 100*(ntimes/LEN); nl++) {
		for (int j = 0; j < LEN; j++) {
			for (int i = j; i < LEN; i++) {
				buffer[i][j] = b[i][j] + c[i][j];
			}
		}
		dummy(nl);
	}

	stop_counters();
*SCALAR_RES_ADDR = buffer[0][0];
	for (int j = 0; j < LEN; j++) {
		for (int i = j; i < LEN; i++) {
			reg_ptr[i][j] = buffer[i][j];
		}
	}

	complete_results(__func__);

}

