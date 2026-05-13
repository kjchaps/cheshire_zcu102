#include "test1.h"
#define SCALAR_RES_ADDR ((volatile float *)0x9FFF0030ULL)

void test1(uint32_t LEN, uint32_t ntimes) {
    TYPE * restrict a = src();
    const TYPE * restrict b = src();

    volatile TYPE *reg_ptr = a;
    TYPE buffer[LEN];
    float sum;

    send_setup_info(LEN, __func__);

    start_counters();

    for (int nl = 0; nl < ntimes * 10; nl++) {
        sum = (float)0.;
        for (int i = 0; i < LEN; i++) {
            sum += a[i];
        }
        dummy(nl);
    }

    stop_counters();

    *SCALAR_RES_ADDR = sum;

    for (uint32_t i = 0; i < LEN; i++) {
        reg_ptr[i] = buffer[i];
    }

    complete_results(__func__);
}



