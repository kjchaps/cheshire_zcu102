#include <xil_types.h>
#include "ff.h"
#include "xil_printf.h"
#include <xstatus.h>
#include "xil_cache.h"
#include "sdCard_new.h"


void full_array_setup(char * test_name, char * ew_full, uint32_t arrLen, int32_t stride);

uint64_t check_s311_float(uint32_t LEN); 