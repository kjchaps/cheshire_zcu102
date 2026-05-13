
#include "init_tsvc.h"
#include "xil_types.h"
#include "sdCard_new.h"

#define WRITE_ZERO  0
#define WRITE_ONES  1
#define WRITE_RAND  2

#define SPECIAL_NONE              0
#define SPECIAL_SET_LAST_NEG_ONE  1
#define SPECIAL_SET_FIRST_ONE     2

void full_array_setup(char * test_name, char * ew_full, uint32_t arrLen, int32_t stride){

    xil_printf("ew_full_tsvc = %s\r\n", ew_full);


    reset_array_allocator();
    if(strcmp(test_name, "rvvTest") == 0){ //fix rvvTest on Cheshire later
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s000") == 0){
        setup_1D('X', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('Y', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s121") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s131") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s162") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s171") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s311") == 0){
        setup_1D('a', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s211") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('e', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s243") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('e', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s251") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s321") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s341") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s351") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); //later make SPECIAL_SET_FIRST_ONE? or keep as 1s
    }
    else if(strcmp(test_name, "s1281") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('e', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
    }
    

    
    //tsvc

    else if(strcmp(test_name, "s111") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s1111") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "s112") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s1112") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }
    else if(strcmp(test_name, "s121") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }
    /*
    else if(strcmp(test_name, "s1232") == 0){
    }*/
    else if(strcmp(test_name, "s131") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }



    

    //must set last element to -1
    else if(strcmp(test_name, "s331") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_SET_LAST_NEG_ONE); 
    }
    /*
    else if(strcmp(test_name, "s421") == 0){
    }*/
    else if(strcmp(test_name, "s491") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('c', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE);
        setup_1D('d', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE);
    }
    else if(strcmp(test_name, "vag") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_RAND, ew_full, arrLen, SPECIAL_NONE); 
    }



    else if(strcmp(test_name, "test1") == 0){
        setup_1D('a', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
    }


//////////////////////////////////////////////////////
/////////////// Put Custom Inits Below ///////////////
//////////////////////////////////////////////////////




    else {
        setup_1D('a', WRITE_ONES, ew_full, arrLen, SPECIAL_NONE); 
        setup_1D('b', WRITE_ZERO, ew_full, arrLen, SPECIAL_NONE); 
     }
}















#define IK_BASE 0x481000000ULL
#define ARR_BASE_PTR_ADDR 0x49EEE0000ULL


uint64_t check_s311_float(uint32_t LEN) {
    uint32_t ntimes = 20;

    volatile uint32_t *ptr_addr = (volatile uint32_t *)(uintptr_t)ARR_BASE_PTR_ADDR;
    uint32_t val = *ptr_addr;  
    uint64_t new_ptr_val = ((uint64_t)0x4 << 32) | val;
    xil_printf("Constructed pointer = 0x%llX\r\n", new_ptr_val);

    volatile float *a = (volatile float *)(uintptr_t)new_ptr_val;

    float buffer[LEN];
    for (uint32_t i = 0; i < LEN; i++) {
        memcpy(&buffer[i], &a[i], sizeof(float));
    }

   
    uint32_t first_raw;
    memcpy(&first_raw, &buffer[0], sizeof(uint32_t));
    xil_printf("First element buffer[0] raw hex = 0x%08X\r\n", first_raw);

    uint64_t sum = 0;
    for (uint32_t nl = 0; nl < ntimes * 10; nl++) {
        for (uint32_t i = 0; i < LEN; i++) {
            uint32_t raw;
            memcpy(&raw, &buffer[i], sizeof(uint32_t));
            sum += raw;
        }
    }

    xil_printf("Sum of all elements (raw hex) = 0x%llX\r\n", sum);
    return sum;
}