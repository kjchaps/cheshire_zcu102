/******************************************************************************
 * Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "sdCard_new.h"
#include "xgpiops.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include <stdint.h> /* uintptr_t */
#include <stdio.h>
#include <string.h>
#include "init_tsvc.h" 
#include "xil_io.h"

uint32_t test_counter;
#define INIT_ADDR 0x49EEC00F0ULL
#define NAME_ADDR 0x49EEC0100ULL
#define EW_ADDR 0x49EEC0110ULL
#define LEN_ADDR 0x49EEC0120ULL





int main(void) {
  char *in_file = "top.bin"; // name of RISC-V binary
  setup(in_file);

  while (1) {
    test_counter = test_counter + 1;
    xil_printf("%d\n", test_counter);
    uint32_t finalRun = runTest(test_counter);
    if (finalRun)
      break;
  }

  print(
      "Benchmarks Complete. \r\nCheck Output File and UART Output for Results");
  while (1) {
  }
  cleanup_platform();
  return 0;
}
