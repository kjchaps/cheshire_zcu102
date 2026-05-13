#include "s351.h"
#define SCALAR_RES_ADDR ((volatile uint32_t *)0x9FFF0030ULL)

void s351(uint32_t LEN,  uint32_t ntimes){
        send_setup_info(LEN, __func__);

        TYPE * restrict a = src();
        TYPE * restrict b = src();
	TYPE * restrict c = src();

        volatile TYPE *reg_ptr = a;
        TYPE buffer[LEN];

        start_counters();

	float alpha = c[0];
   // for (int nl = 0; nl < 8*ntimes; nl++) {
        for (int i = 0; i < LEN; i += 5) {
            buffer[i] = alpha * b[i]; // += alpha * b[i];
            //buffer[i + 1] += alpha * b[i + 1];
           // buffer[i + 2] += alpha * b[i + 2];
           // buffer[i + 3] += alpha * b[i + 3];
            //buffer[i + 4] += alpha * b[i + 4];
        }
       // dummy(nl);
    //}


        stop_counters();
        *SCALAR_RES_ADDR = 0xCCDD;

        for (uint32_t i = 0; i < LEN; i++) {
                reg_ptr[i] = buffer[i];
        }
	*SCALAR_RES_ADDR = 0xAABB; 
        complete_results(__func__);
        *SCALAR_RES_ADDR = 0xEEFF;

}



