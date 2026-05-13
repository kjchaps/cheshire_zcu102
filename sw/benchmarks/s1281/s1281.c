#include "s1281.h"

void s1281(uint32_t LEN, uint32_t ntimes){
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
	
	float x =0;
        for (int nl = 0; nl < 4*ntimes; nl++) {
                for (int i = 0; i < LEN; i++) {
			x = buffer_b[i]*c[i]+buffer_a[i]*d[i]+e[i];
                        buffer_a[i] = x - (float)1.0;
                        buffer_b[i] = x;
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

