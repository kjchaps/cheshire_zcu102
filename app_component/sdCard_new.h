#include <xil_types.h>
#include "ff.h"
#include "xil_printf.h"
#include <xstatus.h>
#include "xil_cache.h"

int SD_Init();
int SD_Eject();
FIL* openFile(char *FileName,char mode);
u32 closeFile(FIL* fptr);
int readFile(FIL *fil, u32 DestinationAddress);
int writeFile(FIL* fptr, u32 size, u64 SourceAddress);
void elf_file_to_dram(const char *path, uintptr_t dram_base, uint32_t dram_offset);
void bin_file_to_dram(const char *path, uintptr_t dram_base, uint32_t dram_offset);

// testing functions
static void list_dir(const char *path);
void test_list_sd(void);
void test_read_sd(int Status);
void test_write_sd(int Status);
void test_read_file(const char *path);
void string_write_read(UINTPTR base_addr, UINTPTR offset);

// cheshire functions
void start_cheshire(UINTPTR base_addr, UINTPTR scratch_offset, UINTPTR ddr_offset);
void cheshire_complete(UINTPTR base_addr, UINTPTR scratch_offset);
void read_cheshire_memory_dump(UINTPTR end_addr, UINTPTR dump_offset, const char *input_name);
void send_pl_reset_pulse_us(uint32_t low_time_us);
inline void dsb_sy(void) ;
void dram_print_first16(uintptr_t dram_base, uint32_t dram_offset); 
void dram_print_hex(uintptr_t dram_base, uint32_t dram_offset);
void write_read_deadbeef(UINTPTR base_addr, UINTPTR offset); 

void writeTrigger(UINTPTR base_addr);
void writeCounter(UINTPTR start_addr, UINTPTR end_addr); 
void checkCheshireDone(UINTPTR base_addr); 
uint32_t checkELFDone(UINTPTR base_addr); 

void dram_print_benchmark_result(uintptr_t start_addr, size_t test_num); 
void clearTriggers(UINTPTR base_addr); 
void write_result(uintptr_t start_addr, uint32_t arrSize);

void write_read_pattern(UINTPTR base_addr, uint32_t pattern);
void write_test_array_1D(UINTPTR base_addr, uint32_t length, uint32_t mode, uint32_t width, uint32_t stride); 
void setup_array(char src_id, uint32_t valToWrite, const char * ew_full, uint32_t size, uint32_t stride, uint32_t array_type,  char *testname, int print_to_file);
uint32_t runTest(uint32_t test_num, int print_to_file); 
void setup(const char *in_file); 
uint32_t calc_arr_end(uint32_t ew, uint32_t size, uint32_t type); 
void reset_array_allocator();

void setup_1D(char src_id, uint32_t valToWrite, const char * ew_full, uint32_t size,uint32_t special_rule, char *testname, int print_to_file);
void apply_special_rule(UINTPTR base_addr,uint32_t size,uint32_t ew,uint32_t special_rule); 
void write_test_array_1D_u(UINTPTR base_addr, uint32_t length, uint32_t mode, uint32_t width, uint32_t stride); 
void write_test_array_1D_fp(UINTPTR base_addr, uint32_t length, uint32_t mode, uint32_t width, uint32_t stride); 

void write_labeled_test_array_1D_fp_to_mem_and_txt(UINTPTR base_addr,
                                                   uint32_t length,
                                                   uint32_t mode,
                                                   uint32_t width,
                                                   uint32_t stride,
                                                   const char *file_stem,
                                                   char array_name); 