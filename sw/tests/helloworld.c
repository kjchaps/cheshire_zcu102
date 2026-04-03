// Copyright 2023 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Nicole Narr <narrn@student.ethz.ch>
// Christopher Reinwardt <creinwar@student.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

#include "regs/cheshire.h"
#include "dif/clint.h"
#include "dif/uart.h"
#include "params.h"
#include "util.h"


#define DONE_ADDR    ((volatile uint32_t *)0x9EEC0020ULL)
#define D_ADDR    ((volatile uint32_t *)0x8EEC0020ULL)
 

static inline void fence_rw_rw(void) {
  __asm__ volatile ("fence" ::: "memory");
}


int main(void) {

*D_ADDR = 0xAAAA;
    volatile uint32_t *p   = (volatile uint32_t *)(uintptr_t)0x9EED0000ULL;
     volatile uint32_t *end = (volatile uint32_t *)(uintptr_t)0x9FFFF000ULL;


    uint32_t value = 0xAAAA;

    while (p < end) {
        *p++ = value;
        value += 0x1111;

        if (value > 0xFFFF) {
            value = 0xAAAA;
        }
    }


*(volatile uint32_t *)0x9EEC0020 = 0xf005ba11;
     fence_rw_rw();

while (1) { }

    return 0;
}
