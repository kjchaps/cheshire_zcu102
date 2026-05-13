#include "s000.h"

void s000(uint32_t LEN,  uint32_t ntimes){
	send_setup_info(LEN, __func__);
        
	TYPE * restrict X = src(); 
	TYPE * restrict Y = src(); 

	volatile TYPE *reg_ptr = X;
	TYPE buffer[LEN];
	
	start_counters();
	
	for (int nl = 0; nl < 2*ntimes; nl++) {
		for (int i = 0; i < LEN; i++) {
			buffer[i] = Y[i] + 1;
		}
		dummy(nl);
	}
	
	stop_counters();
	
	for (uint32_t i = 0; i < LEN; i++) {
		reg_ptr[i] = buffer[i]; 
	}
	
	complete_results(__func__);

}




