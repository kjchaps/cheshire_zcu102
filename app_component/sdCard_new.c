#include "sdCard_new.h"
#include "init_tsvc.h"
#include "xil_types.h"
#include <stdlib.h>
#include <xil_printf.h>
#include <stdio.h>

static FATFS fatfs;
///////////////////////////////////////////
// Base Addresses
#define BUFFER_BASE_ADDR 0x481000000ULL
#define ARR_MAX_ADDR 0x49ED00000ULL
#define RESULTS_ADDR 0x49EED0000ULL
#define ARR_BASE_PTR_ADDR 0x49EEE0000ULL
#define NAME_ADDR 0x49EEC0100ULL
#define EW_ADDR 0x49EEC0110ULL
#define LEN_ADDR 0x49EEC0120ULL
// Polling Addresses
#define BOOT_ADDR 0x49EEC0000ULL
#define DONE_ADDR 0x49EEC0020ULL
#define START_ADDR 0x49EEC0030ULL
#define INIT_ADDR 0x49EEC0040ULL
#define ARR_ADDR 0x49EEC0050ULL
// Array base address stride
#define SRC_REG_STRIDE 0x10ULL
///////////////////////////////////////////
// Array value types
#define WRITE_ZERO 0
#define WRITE_ONES 1
#define WRITE_RAND 2








int SD_Init() {
  FRESULT rc;
  TCHAR *Path = "0:/";
  rc = f_mount(&fatfs, Path, 0);
  if (rc) {
    xil_printf(" ERROR : f_mount returned %d\r\n", rc);
    return XST_FAILURE;
  }
  return XST_SUCCESS;
}

int SD_Eject() {
  FRESULT rc;
  TCHAR *Path = "0:/";
  rc = f_mount(0, Path, 0);
  if (rc) {
    xil_printf(" ERROR : f_mount returned %d\r\n", rc);
    return XST_FAILURE;
  }
  return XST_SUCCESS;
}

int readFile(FIL *fil, u32 DestinationAddress) {

  FRESULT rc;
  UINT br;
  u32 file_size;

  file_size = f_size(fil);
  rc = f_lseek(fil, 0);
  if (rc) {
    xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
    return XST_FAILURE;
  }
  rc = f_read(fil, (void *)(uintptr_t)DestinationAddress, file_size, &br);
  if (rc) {
    xil_printf(" ERROR : f_read returned %d\r\n", rc);
    return XST_FAILURE;
  }
  Xil_DCacheFlush();
  return file_size;
}

u32 closeFile(FIL *fptr) {
  FRESULT rc; // FRESULT variable
  rc = f_close(fptr);
  if (rc) {
    xil_printf(" ERROR : f_close returned %d\r\n", rc);
    return XST_FAILURE;
  }
  return XST_SUCCESS;
}

FIL *openFile(char *FileName, char mode) {
  static FIL fil; // File instance
  FRESULT rc;     // FRESULT variable
  if (mode == 'r')
    rc = f_open(&fil, FileName, FA_READ);
  else if (mode == 'w') {
    rc = f_open(&fil, (char *)FileName, FA_CREATE_NEW | FA_WRITE); // f_open
    if (rc != FR_OK) {         // file already exists
      rc = f_unlink(FileName); // delete the file;
      rc = f_open(&fil, (char *)FileName, FA_CREATE_NEW | FA_WRITE); // f_open
    }
  } else if (mode == 'a') {
    rc = f_open(&fil, (char *)FileName, FA_OPEN_ALWAYS | FA_WRITE); // f_open
    if (rc != FR_OK) { // file doesn't exists
      rc = f_open(&fil, (char *)FileName, FA_CREATE_NEW | FA_WRITE); // f_open
    } else
      rc = f_lseek(&fil, f_size(&fil));
  }
  if (rc) {
    xil_printf(" ERROR : f_open returned %d\r\n", rc);
    return (FIL *)0;
  }
  return &fil;
}

int writeFile(FIL *fptr, u32 size, u64 SourceAddress) {
  UINT btw;
  FRESULT rc; // FRESULT variable
  // xil_printf("Writing file %0x size %0x address
  // %0d",fptr->fptr,size,SourceAddress);
  rc = f_write(fptr, (const void *)(uintptr_t)SourceAddress, size, &btw);
  if (rc) {
    xil_printf(" ERROR : f_write returned %d\r\n", rc);
    return XST_FAILURE;
  }
  return btw;
}

void elf_file_to_dram(const char *path, uintptr_t dram_base,
                      uint32_t dram_offset) {
  int Status = SD_Init();
  if (Status == XST_FAILURE) {
    xil_printf("SD card init failed\r\n");
    return;
  }

  FIL fp;
  FRESULT fr = f_open(&fp, path, FA_READ);
  if (fr != FR_OK) {
    xil_printf("f_open('%s') failed, fr=%d\r\n", path, fr);
    SD_Eject();
    return;
  }

  static uint8_t buf[4096];
  UINT br = 0;
  uint32_t total = 0;

  volatile uint8_t *dst =
      (volatile uint8_t *)(dram_base + (uintptr_t)dram_offset);

  while (1) {
    fr = f_read(&fp, buf, (UINT)sizeof(buf), &br);
    if (fr != FR_OK) {
      xil_printf("f_read failed, fr=%d\r\n", fr);
      break;
    }
    if (br == 0) {
      break; // EOF
    }

    for (UINT i = 0; i < br; i++) {
      dst[total + i] = buf[i];
    }
    total += (uint32_t)br;
  }

  uint32_t fileSize = (uint32_t)f_size(&fp);

  f_close(&fp);
  SD_Eject();

  xil_printf("File: %s\r\n", path);
  //xil_printf("DRAM addr: 0x%08lx (base 0x%08lx + offset 0x%08lx)\r\n",
  //           (unsigned long)(dram_base + (uintptr_t)dram_offset),
  //           (unsigned long)dram_base, (unsigned long)dram_offset);
  //xil_printf("Total bytes written to DRAM: %lu\r\n", (unsigned long)total);
  //xil_printf("Reported file size: %lu\r\n", (unsigned long)fileSize);

  if (total != fileSize) {
    xil_printf("Warning: bytes written != file size\r\n");
  }

  // ----------------------------
  // Verification: spot check DRAM
  // ----------------------------
  {
    uint32_t dump_len = 16;
    if (total < dump_len)
      dump_len = total;

    //xil_printf("First %lu bytes at DRAM destination:\r\n",
    //           (unsigned long)dump_len);
    //for (uint32_t i = 0; i < dump_len; i++) {
    //  xil_printf("%02x ", (unsigned int)dst[i]);
    //}
    xil_printf("\r\n");

    if (dump_len >= 4 && dst[0] == 0x7F && dst[1] == 'E' && dst[2] == 'L' &&
        dst[3] == 'F') {
      xil_printf("ELF magic check: OK (7f 45 4c 46)\r\n");
    } else if (dump_len >= 4) {
      xil_printf("ELF magic check: FAIL (expected 7f 45 4c 46)\r\n");
    } else {
      xil_printf("ELF magic check: SKIPPED (file too small)\r\n");
    }
  }
}

void bin_file_to_dram(const char *path, uintptr_t dram_base,
                      uint32_t dram_offset) {
  int Status = SD_Init();
  if (Status == XST_FAILURE) {
    xil_printf("SD card init failed\r\n");
    return;
  }

  FIL fp;
  FRESULT fr = f_open(&fp, path, FA_READ);
  if (fr != FR_OK) {
    xil_printf("f_open('%s') failed, fr=%d\r\n", path, fr);
    SD_Eject();
    return;
  }

  static uint8_t buf[4096];
  UINT br = 0;
  uint32_t total = 0;

  volatile uint8_t *dst =
      (volatile uint8_t *)(dram_base + (uintptr_t)dram_offset);

  while (1) {
    fr = f_read(&fp, buf, (UINT)sizeof(buf), &br);
    if (fr != FR_OK) {
      xil_printf("f_read failed, fr=%d\r\n", fr);
      break;
    }
    if (br == 0) {
      break; // EOF
    }

    for (UINT i = 0; i < br; i++) {
      dst[total + i] = buf[i];
    }
    total += (uint32_t)br;
  }

  uint32_t fileSize = (uint32_t)f_size(&fp);

  f_close(&fp);
  SD_Eject();

  xil_printf("File: %s\r\n", path);
  //xil_printf("DRAM addr: 0x%08lx (base 0x%08lx + offset 0x%08lx)\r\n",
  //           (unsigned long)(dram_base + (uintptr_t)dram_offset),
  //           (unsigned long)dram_base, (unsigned long)dram_offset);
  //xil_printf("Total bytes written to DRAM: %lu\r\n", (unsigned long)total);
  //xil_printf("Reported file size: %lu\r\n", (unsigned long)fileSize);

  if (total != fileSize) {
    xil_printf("Warning: bytes written != file size\r\n");
  }

  // ----------------------------
  // Verification: spot check DRAM
  // ----------------------------
  {
    uint32_t dump_len = 16;
    if (total < dump_len)
      dump_len = total;

    //xil_printf("First %lu bytes at DRAM destination:\r\n",
    //           (unsigned long)dump_len);
    //for (uint32_t i = 0; i < dump_len; i++) {
    //  xil_printf("%02x ", (unsigned int)dst[i]);
    //}
    xil_printf("\r\n");
  }

  UINTPTR addr = (UINTPTR)(dram_base + (uintptr_t)dram_offset);
  Xil_DCacheFlushRange(addr, total);
}

void dram_print_first16(uintptr_t dram_base, uint32_t dram_offset) {
  volatile const uint8_t *dst =
      (volatile const uint8_t *)(dram_base + (uintptr_t)dram_offset);

  xil_printf("DRAM addr: 0x%08lx (base 0x%08lx + offset 0x%08lx)\r\n",
             (unsigned long)(dram_base + (uintptr_t)dram_offset),
             (unsigned long)dram_base, (unsigned long)dram_offset);

  xil_printf("First 32 bytes at DRAM destination:\r\n");
  for (uint32_t i = 0; i < 32; i++) {
    xil_printf("%02x ", (unsigned int)dst[i]);
  }
  xil_printf("\r\n");
}

#include <inttypes.h>
#include <stdint.h>

void dram_print_hex(uintptr_t dram_base, uint32_t dram_offset) {
  uintptr_t addr = dram_base + (uintptr_t)dram_offset;
  volatile const uint8_t *dst = (volatile const uint8_t *)addr;

  xil_printf("hex:\r\n");
  for (uint32_t i = 0; i < 32; i++) {
    xil_printf("%02x ", (unsigned int)dst[i]);
  }
  xil_printf("\r\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// for testing
static void list_dir(const char *path) {
  DIR dir;
  FILINFO fno;
  FRESULT fr;

  fr = f_opendir(&dir, path);
  xil_printf("Listing %s, fr=%d\r\n", path, fr);
  if (fr != FR_OK)
    return;

  while (1) {
    fr = f_readdir(&dir, &fno);
    if (fr != FR_OK) {
      xil_printf("f_readdir fr=%d\r\n", fr);
      break;
    }
    if (fno.fname[0] == 0) {
      break; // end of dir
    }

    xil_printf("%s%s\r\n", (fno.fattrib & AM_DIR) ? "<DIR> " : "      ",
               fno.fname);
  }

  f_closedir(&dir);
}

void test_list_sd(void) {
  if (SD_Init() == XST_FAILURE) {
    xil_printf("SD card init failed\r\n");
    return;
  }

  list_dir("0:/"); // root

  SD_Eject();
}

void test_write_sd(int Status) {
  Status = SD_Init();
  if (Status == XST_FAILURE)
    xil_printf("SD card init failed");

  xil_printf("SD_Init OK\r\n");
  FIL *fp = openFile("log.txt", 'a');
  if (fp == 0)
    xil_printf("File opening failed\n\r");
  xil_printf("openFile append OK\r\n");

  const char *line1 = "Cheshire\r\n";
  const char *line2 = "Basic Test Here\r\n";

  Status = writeFile(fp, (u32)strlen(line1), (u32)(uintptr_t)line1);
  if (Status == XST_FAILURE)
    xil_printf("writeFile line1 failed\r\n");
  xil_printf("writeFile line1 OK\r\n");

  Status = writeFile(fp, (u32)strlen(line2), (u32)(uintptr_t)line2);
  if (Status == XST_FAILURE)
    xil_printf("writeFile line2 failed\r\n");
  xil_printf("writeFile line2 OK\r\n");

  Status = (int)closeFile(fp);
  if (Status == XST_FAILURE)
    xil_printf("closeFile(append) failed\r\n");
  xil_printf("closeFile append OK\r\n");

  Status = SD_Eject();
  if (Status == XST_FAILURE)
    xil_printf("SD_Eject failed\r\n");

  xil_printf("SD_Eject OK, safe to remove SD card\r\n");
}
void test_read_sd(int Status) {
  Status = SD_Init();
  if (Status == XST_FAILURE)
    xil_printf("SD card init failed");
  FIL *fp = openFile("log.txt", 'r');
  if (fp == 0)
    xil_printf("openFile('log.txt','r') failed\r\n");

  xil_printf("openFile read OK\r\n");

  static char readBuf[2048];

  int fileSize = readFile(fp, (u32)(uintptr_t)readBuf);
  if (fileSize < 0) {
    xil_printf("readFile failed\r\n");
    closeFile(fp);
  }
  xil_printf("readFile OK, bytes=%d\r\n", fileSize);

  Status = (int)closeFile(fp);
  if (Status == XST_FAILURE)
    xil_printf("closeFile(read) failed\r\n");

  xil_printf("closeFile read OK\r\n");

  if (fileSize >= (int)sizeof(readBuf)) {
    xil_printf("Warning, file truncated to fit buffer\r\n");
    fileSize = (int)sizeof(readBuf) - 1;
  }
  readBuf[fileSize] = '\0';

  xil_printf("Read back content:\r\n%s\r\n", readBuf);
  Status = SD_Eject();
  if (Status == XST_FAILURE)
    xil_printf("SD_Eject failed\r\n");

  xil_printf("SD_Eject OK, safe to remove SD card\r\n");
}
void test_read_file(const char *path) {
  int Status = SD_Init();
  if (Status == XST_FAILURE) {
    xil_printf("SD card init failed\r\n");
    return;
  }

  FIL fp;
  FRESULT fr = f_open(&fp, path, FA_READ);
  if (fr != FR_OK) {
    xil_printf("f_open('%s') failed, fr=%d\r\n", path, fr);
    SD_Eject();
    return;
  }

  static uint8_t buf[4096];
  UINT br = 0;
  uint32_t total = 0;

  while (1) {
    fr = f_read(&fp, buf, (UINT)sizeof(buf), &br);
    if (fr != FR_OK) {
      xil_printf("f_read failed, fr=%d\r\n", fr);
      break;
    }
    if (br == 0) {
      xil_printf("EOF\r\n");
      break;
    }
    total += (uint32_t)br;
  }

  uint32_t fileSize = (uint32_t)f_size(&fp);

  f_close(&fp);
  SD_Eject();

  xil_printf("File: %s\r\n", path);
  xil_printf("Total bytes read from file read: %lu\r\n", (unsigned long)total);
  xil_printf("Reported file size from Directory: %lu\r\n",
             (unsigned long)fileSize);
}

void string_write_read(UINTPTR base_addr, UINTPTR offset) {
  volatile char *dst = (volatile char *)(base_addr + offset);

  const char msg[] = "KATY_DDR_TEST_TO_OFFSET";
  unsigned int i;

  for (i = 0; i < sizeof(msg); i++)
    dst[i] = msg[i];

  //xil_printf("Readback: ");
  //for (i = 0; i < sizeof(msg); i++)
  //  xil_printf("%c", dst[i]);
  //xil_printf("\r\n");

  for (i = 0; i < sizeof(msg); i++) {
    if (dst[i] != msg[i]) {
      //xil_printf("Mismatch at %u, wrote 0x%02x read 0x%02x\r\n", i,
      //           (unsigned char)msg[i], (unsigned char)dst[i]);
      return;
    }
  }
  //xil_printf("String compare PASSED\r\n");
}

void write_read_deadbeef(UINTPTR base_addr, UINTPTR offset) {
  volatile uint8_t *dst = (volatile uint8_t *)(base_addr + offset);

  const uint8_t pattern[4] = {0x80, 0x00, 0xEC, 0x8E};

  // Write
  for (unsigned int i = 0; i < 4; i++) {
    dst[i] = pattern[i];
  }

  // Readback print
  //xil_printf("Readback @ 0x%08lx: ", (unsigned long)(base_addr + offset));
  //for (unsigned int i = 0; i < 4; i++) {
  //  xil_printf("%02x ", (unsigned int)dst[i]);
  //}
  //xil_printf("\r\n");

  // Verify
  for (unsigned int i = 0; i < 4; i++) {
    if (dst[i] != pattern[i]) {
     // xil_printf("Mismatch at byte %u, wrote 0x%02x read 0x%02x\r\n", i,
     //            (unsigned int)pattern[i], (unsigned int)dst[i]);
      return;
    }
  }

  //xil_printf("DEADBEEF compare PASSED\r\n");
}

///////////////////////////////////////////////////////////////
// More custom, don't touch
// for fake scratch registers for bootrom and memory dump
// PL_DDR4_BASE   ((UINTPTR)0x0400000000ULL)
// TEST_OFFSET    (0x2000U)

// Must add to main:
//// PL_DDR_SCRATCH_OFFSET (0x1000U)
//// PL_DDR_END ((UINTPTR)0x0420000000ULL)
//// PL_DDR_MEMORY_DUMP_OFFSET_SUBTRACT (0x2000U)

// volatile uint32_t *scratch = reg32(&__base_dram, SCRATCH_OFFSET_1000);
// scratch[0] provides an entry point, scratch[1] a start address high word
// we wait for while (!(scratch[2] & 2)) {} to break
///////////////////////////////////////////////////////////////

void start_cheshire(UINTPTR base_addr, UINTPTR scratch_offset,
                    UINTPTR ddr_offset) {
  const UINTPTR scratch_addr = base_addr + scratch_offset;
  const UINTPTR ddr_addr = base_addr + ddr_offset;

  volatile u32 *regs = (volatile u32 *)(uintptr_t)scratch_addr;

  xil_printf(
      "start_cheshire: base=0x%08lx scratch_off=0x%08lx ddr_off=0x%08lx\r\n",
      (unsigned long)base_addr, (unsigned long)scratch_offset,
      (unsigned long)ddr_offset);

  xil_printf("start_cheshire: scratch_addr=0x%08lx entry_addr=0x%08lx\r\n",
             (unsigned long)scratch_addr, (unsigned long)base_addr,
             (unsigned long)ddr_addr);

  for (unsigned i = 0; i < 6; ++i) {
    regs[i] = 0u;
  }

  const u64 entry = (u64)ddr_addr;
  regs[0] = (u32)(entry & 0xFFFFFFFFu);
  regs[1] = (u32)((entry >> 32) & 0xFFFFFFFFu);

  // asm volatile ("fence iorw, iorw" ::: "memory");
  regs[2] = 2u;

  // asm volatile ("fence iorw, iorw" ::: "memory");
}

void cheshire_complete(UINTPTR base_addr, UINTPTR scratch_offset) {
  UINTPTR addr = base_addr + scratch_offset;
  volatile u32 *scratch = (volatile u32 *)(uintptr_t)addr;
/*
  xil_printf("cheshire_complete: base=%p scratch_off=0x%lx scratch_addr=%p\r\n",
             (void *)(uintptr_t)base_addr, (unsigned long)scratch_offset,
             (void *)(uintptr_t)addr);

  xil_printf("cheshire_complete: scratch=%p scratch[4]_addr=%p "
             "scratch[4]_value=%lu\r\n",
             (void *)scratch, (void *)&scratch[4], (unsigned long)scratch[4]);
*/
  const u32 log_every = 100000u;
  const u32 max_spins = 2000000u;
  u32 spins = 0;

  for (;;) {
    Xil_DCacheInvalidateRange((INTPTR)&scratch[4], sizeof(u32));
    u32 v = scratch[4];

    /*if ((spins % log_every) == 0u) {
      xil_printf("cheshire_complete: checked scratch[4]=%lu (spins=%lu)\r\n",
                 (unsigned long)v, (unsigned long)spins);
    }*/

    if (v == 7u) {
      //xil_printf(
      //    "cheshire_complete: saw scratch[4]==7, setting ack scratch[4]=0\r\n");
      scratch[4] = 0u;
      Xil_DCacheFlushRange((INTPTR)&scratch[4], sizeof(u32));
      return;
    }

    spins++;
    if (spins >= max_spins) {
      //xil_printf("cheshire_complete: timeout waiting for scratch[4]==2 "
      //           "(spins=%lu)\r\n",
      //           (unsigned long)spins);
      return;
    }
  }
}

void read_cheshire_memory_dump(UINTPTR end_addr, UINTPTR dump_offset,
                               const char *input_name) {
  int Status;

  UINTPTR start_addr = end_addr - dump_offset;
  UINTPTR size = dump_offset;

  /* build output filename: <basename>_dump.bin */
  char out_name[64];
  u32 i = 0;

  if (input_name == 0 || input_name[0] == 0) {
    strcpy(out_name, "memdump_dump.bin");
  } else {
    while (input_name[i] != 0 && input_name[i] != '.' &&
           i < (sizeof(out_name) - sizeof("_dump.bin"))) {
      out_name[i] = input_name[i];
      i++;
    }
    out_name[i] = 0;

    if (i == 0) {
      strcpy(out_name, "memdump_dump.bin");
    } else {
      strcpy(&out_name[i], "_dump.bin");
    }
  }

  xil_printf("Dump range: start=0x%08lx size=0x%08lx (%lu bytes)\r\n",
             (unsigned long)start_addr, (unsigned long)size,
             (unsigned long)size);

  /* Make sure CPU reads the latest DDR contents */
  Xil_DCacheInvalidateRange((INTPTR)start_addr, (u32)size);

  /* Print first 16 bytes */
  {
    volatile const u8 *p = (volatile const u8 *)(uintptr_t)start_addr;
    u32 n = (size >= 16u) ? 16u : (u32)size;

    xil_printf("First %lu bytes @ 0x%08lx:\r\n", (unsigned long)n,
               (unsigned long)start_addr);
    for (u32 j = 0; j < n; j++) {
      xil_printf("%02x ", (unsigned)p[j]);
    }
    xil_printf("\r\n");
  }

  Status = SD_Init();
  if (Status == XST_FAILURE)
    xil_printf("SD card init failed\r\n");
  xil_printf("SD_Init OK\r\n");

  FIL *fp = openFile(out_name, 'w');
  if (fp == 0)
    xil_printf("File opening failed\n\r");
  xil_printf("openFile write OK (%s)\r\n", out_name);

  Status = writeFile(fp, (u32)size, (u32)(uintptr_t)start_addr);
  if (Status == XST_FAILURE)
    xil_printf("writeFile failed\r\n");
  xil_printf("writeFile OK\r\n");

  Status = (int)closeFile(fp);
  if (Status == XST_FAILURE)
    xil_printf("closeFile failed\r\n");
  xil_printf("closeFile OK\r\n");

  Status = SD_Eject();
  if (Status == XST_FAILURE)
    xil_printf("SD_Eject failed\r\n");
  xil_printf("SD_Eject OK, safe to remove SD card\r\n");
}

#define GPIO_BASE 0xFF0A0000u
#define GPIO_DATA_5 0x0054u
#define GPIO_DIRM_5 0x0344u
#define GPIO_OEN_5 0x0348u

#define PL_RESETN0_MASK 0x80000000u // Bank5 bit31, EMIO[95]

void dsb_sy(void) { asm volatile("dsb sy" ::: "memory"); }

void send_pl_reset_pulse_us(uint32_t low_time_us) {
  uint32_t v;

  // Configure as output
  v = Xil_In32(GPIO_BASE + GPIO_DIRM_5);
  Xil_Out32(GPIO_BASE + GPIO_DIRM_5, v | PL_RESETN0_MASK);

  v = Xil_In32(GPIO_BASE + GPIO_OEN_5);
  Xil_Out32(GPIO_BASE + GPIO_OEN_5, v | PL_RESETN0_MASK);

  dsb_sy();

  // Ensure deasserted first (active low reset)
  v = Xil_In32(GPIO_BASE + GPIO_DATA_5);
  Xil_Out32(GPIO_BASE + GPIO_DATA_5, v | PL_RESETN0_MASK);
  dsb_sy();
  usleep(10);

  xil_printf("Sending reset...\r\n");

  // Assert reset (drive low)
  v = Xil_In32(GPIO_BASE + GPIO_DATA_5);
  Xil_Out32(GPIO_BASE + GPIO_DATA_5, v & ~PL_RESETN0_MASK);
  dsb_sy();

  usleep(low_time_us);

  // Deassert reset (drive high)
  v = Xil_In32(GPIO_BASE + GPIO_DATA_5);
  Xil_Out32(GPIO_BASE + GPIO_DATA_5, v | PL_RESETN0_MASK);
  dsb_sy();

  xil_printf("Reset sent.\r\n");
}

void writeTrigger(UINTPTR base_addr) {
  volatile uint8_t *dst = (volatile uint8_t *)(base_addr);
  const uint8_t pattern[4] = {0xBE, 0xBA, 0xFE, 0xCA};

  // Write
  for (unsigned int i = 0; i < 4; i++) {
    dst[i] = pattern[i];
  }

  // Flush written bytes from DCache
  Xil_DCacheFlushRange(base_addr, 4);

  // Optional barrier
  dsb();

  // Readback print
  //xil_printf("Readback @ 0x%08lx: ", (unsigned long)base_addr);
  //for (unsigned int i = 0; i < 4; i++) {
  //  xil_printf("%02x ", (unsigned int)dst[i]);
  //}
  //xil_printf("\r\n");

  /*
  for (unsigned int i = 0; i < 4; i++) {
    if (dst[i] != pattern[i]) {
      xil_printf("Mismatch at byte %u, wrote 0x%02x read 0x%02x\r\n", i,
                 (unsigned int)pattern[i], (unsigned int)dst[i]);
      return;
    }
  }*/

  //xil_printf("Trigger compare PASSED\r\n");
}

void writeCounter(UINTPTR start_addr, UINTPTR end_addr) {
  volatile uint32_t *p = (volatile uint32_t *)start_addr;
  volatile uint32_t *end = (volatile uint32_t *)end_addr;

  uint32_t counter = 1;

  while (p < end) {
    *p++ = counter++;
  }

  Xil_DCacheFlushRange(start_addr, end_addr - start_addr);
  xil_printf("Arm is done writing the counter. \r\n");
}

void checkCheshireDone(UINTPTR base_addr) {
  volatile u8 *addr = (volatile u8 *)(uintptr_t)base_addr;

  xil_printf("cheshire_complete: base=%p\r\n", (void *)(uintptr_t)base_addr);

  const u32 log_every = 1000000u;
  const u32 max_spins = 20000000u;
  u32 spins = 0;

  for (;;) {
    Xil_DCacheInvalidateRange((INTPTR)addr, 4);

    u8 b0 = addr[0];
    u8 b1 = addr[1];
    u8 b2 = addr[2];
    u8 b3 = addr[3];

    if ((spins % log_every) == 0u) {
      xil_printf("cheshire_complete: checked bytes = {%02X %02X %02X %02X} "
                 "(spins=%lu)\r\n",
                 b0, b1, b2, b3, (unsigned long)spins);
    }

    if (b0 == 0xEF && b1 == 0xBE && b2 == 0xAD && b3 == 0xDE) {
      xil_printf("Cheshire is Done. DEADBEEF read.\r\n");
      return;
    }

    spins++;
    if (spins >= max_spins) {
      xil_printf("cheshire_complete: timeout waiting for DEADBEEF at base_addr "
                 "(spins=%lu)\r\n",
                 (unsigned long)spins);
      return;
    }
  }
}

uint32_t checkELFDone(UINTPTR base_addr) {
  volatile u32 *addr = (volatile u32 *)(uintptr_t)base_addr;

  //xil_printf("cheshire_complete: base=%p\r\n", (void *)(uintptr_t)base_addr);

  const u32 log_every = 10000000u;
  const u32 max_spins = 2000000000u;
  u32 spins = 0;

  for (;;) {
    Xil_DCacheInvalidateRange((INTPTR)addr, 4);

    u32 val = *addr;
    /*
    if ((spins % log_every) == 0u) {
      xil_printf("elf_done: value at %p = 0x%08X (spins=%lu)\r\n", (void *)addr,
                 val, (unsigned long)spins);
    }*/

    if (val == 0xF005BA11u) {
      //xil_printf("Elf is Done, f005ba11 read.\r\n");
      return 0;
    }

    if (val == 0xDECAFBADu) {
      //xil_printf("Elf failed, decafbad read.\r\n");
      return 1;
    }

    spins++;
    if (spins >= max_spins) {
      //xil_printf("elf_done: timeout waiting for f005ba11 or decafbad at "
      //           "base_addr (spins=%lu)\r\n",
      //           (unsigned long)spins);
      return 1;
    }
  }
}


void write_read_pattern(UINTPTR base_addr, uint32_t pattern) {
  volatile uint8_t *dst = (volatile uint8_t *)base_addr;
  uint8_t bytes[4];

  // Convert 0x8EEC0080 into byte order: 0x80, 0x00, 0xEC, 0x8E
  bytes[0] = (uint8_t)(pattern & 0xFF);
  bytes[1] = (uint8_t)((pattern >> 8) & 0xFF);
  bytes[2] = (uint8_t)((pattern >> 16) & 0xFF);
  bytes[3] = (uint8_t)((pattern >> 24) & 0xFF);

  // Write
  for (unsigned int i = 0; i < 4; i++) {
    dst[i] = bytes[i];
  }

  /*
  xil_printf("Readback @ 0x%08lx: ", (unsigned long)base_addr);
  for (unsigned int i = 0; i < 4; i++) {
    xil_printf("%02x ", (unsigned int)dst[i]);
  }
  xil_printf("\r\n");

  
  for (unsigned int i = 0; i < 4; i++) {
    if (dst[i] != bytes[i]) {
      xil_printf("Mismatch at byte %u, wrote 0x%02x read 0x%02x\r\n", i,
                 (unsigned int)bytes[i], (unsigned int)dst[i]);
      return;
    }
  }

  xil_printf("Pattern compare PASSED\r\n");
  */
}

void dram_print_benchmark_result(uintptr_t start_addr, size_t test_num) {
  volatile const uint8_t *base = (volatile const uint8_t *)start_addr;

  volatile const uint8_t *p = base;

  char test_name[9];
  for (int i = 0; i < 8; i++) {
    test_name[i] = (char)p[i];
  }
  test_name[8] = '\0';

  uint32_t minst_lo = ((uint32_t)p[8]) | ((uint32_t)p[9] << 8) |
                      ((uint32_t)p[10] << 16) | ((uint32_t)p[11] << 24);

  uint32_t minst_hi = ((uint32_t)p[12]) | ((uint32_t)p[13] << 8) |
                      ((uint32_t)p[14] << 16) | ((uint32_t)p[15] << 24);

  uint32_t mcycle_lo = ((uint32_t)p[16]) | ((uint32_t)p[17] << 8) |
                       ((uint32_t)p[18] << 16) | ((uint32_t)p[19] << 24);

  uint32_t mcycle_hi = ((uint32_t)p[20]) | ((uint32_t)p[21] << 8) |
                       ((uint32_t)p[22] << 16) | ((uint32_t)p[23] << 24);

  uint64_t minst = ((uint64_t)minst_hi << 32) | minst_lo;
  uint64_t mcycle = ((uint64_t)mcycle_hi << 32) | mcycle_lo;

  //xil_printf("Benchmark record %lu at 0x%08lx:\r\n", (unsigned long)test_num,
  //           (unsigned long)(start_addr));

  //xil_printf("Raw bytes:\r\n");
  //for (int i = 0; i < 24; i++) {
  //  xil_printf("%02x ", (unsigned int)p[i]);
  //}
  xil_printf("\r");

  xil_printf("test=%s, minst=", test_name);
  if (minst <= 0xFFFFFFFFULL) {
    xil_printf("%lu", (unsigned long)minst);
  } else {
    xil_printf("0x%08lx%08lx", (unsigned long)minst_hi,
               (unsigned long)minst_lo);
  }

  xil_printf(", mcycle=");
  if (mcycle <= 0xFFFFFFFFULL) {
    xil_printf("%lu\r\n", (unsigned long)mcycle);
  } else {
    xil_printf("0x%08lx%08lx\r\n", (unsigned long)mcycle_hi,
               (unsigned long)mcycle_lo);
  }

  xil_printf("\r\n");
}

void clearTriggers(UINTPTR base_addr) {
  volatile uint8_t *dst = (volatile uint8_t *)base_addr;

  for (unsigned int i = 0; i < 0x1000; i++) {
    dst[i] = 0x00;
  }

  Xil_DCacheFlushRange(base_addr, 0x1000);
  dsb();

  //xil_printf("Cleared range 0x%08lx to 0x%08lx\r\n", (unsigned long)base_addr,
  //           (unsigned long)(base_addr + 0xFFF));
}

void write_result(uintptr_t start_addr, uint32_t arrSize) {
  volatile const uint8_t *base = (volatile const uint8_t *)start_addr;
  char outbuf[256];

  SD_Init();

  FIL *fp = openFile("out.txt", 'a');
  if (fp == 0) {
    SD_Eject();
    return;
  }

  volatile const uint8_t *p = base;
  
  
  char EW[9];
  memcpy(EW, (const void *)EW_ADDR, 8);
  EW[8] = '\0';


  char test_name[9];
  for (int i = 0; i < 8; i++) {
    test_name[i] = (char)p[i];
  }
  test_name[8] = '\0';

  uint32_t minst_lo = ((uint32_t)p[8]) | ((uint32_t)p[9] << 8) |
                      ((uint32_t)p[10] << 16) | ((uint32_t)p[11] << 24);

  uint32_t minst_hi = ((uint32_t)p[12]) | ((uint32_t)p[13] << 8) |
                      ((uint32_t)p[14] << 16) | ((uint32_t)p[15] << 24);

  uint32_t mcycle_lo = ((uint32_t)p[16]) | ((uint32_t)p[17] << 8) |
                       ((uint32_t)p[18] << 16) | ((uint32_t)p[19] << 24);

  uint32_t mcycle_hi = ((uint32_t)p[20]) | ((uint32_t)p[21] << 8) |
                       ((uint32_t)p[22] << 16) | ((uint32_t)p[23] << 24);

  uint64_t minst = ((uint64_t)minst_hi << 32) | minst_lo;
  uint64_t mcycle = ((uint64_t)mcycle_hi << 32) | mcycle_lo;

  int len;
  if (minst <= 0xFFFFFFFFULL && mcycle <= 0xFFFFFFFFULL) {
    len = sprintf(outbuf, sizeof(outbuf),
                   "test=%s, type=%s, arrSize=%u, minst=%lu, mcycle=%lu\r\n",
                   test_name, EW, arrSize, (unsigned long)minst,
                   (unsigned long)mcycle);
  } else {
    len = sprintf(outbuf, sizeof(outbuf),
                   "test=%s, type=%s, arrSize=%u, minst=0x%08lx%08lx, "
                   "mcycle=0x%08lx%08lx\r\n",
                   test_name, EW, arrSize, (unsigned long)minst_hi,
                   (unsigned long)minst_lo, (unsigned long)mcycle_hi,
                   (unsigned long)mcycle_lo);
  }

  if (len > 0) {
    writeFile(fp, len, (uintptr_t)outbuf);
  }

  closeFile(fp);
  SD_Eject();
}

void write_test_array_1D_u(UINTPTR base_addr,
                         uint32_t length,
                         uint32_t mode,
                         uint32_t width,
                         uint32_t stride)
{
    uint32_t i;
    uint32_t elem_size = width / 8;  // 8→1, 16→2, 32→4, 64→8

    // fallback: no stride = tightly packed

    uint8_t *base = (uint8_t *)base_addr;

    for (i = 0; i < length; i++) {
        uint64_t value;

        // generate value once
        if (mode == WRITE_RAND) {
            value = ((uint64_t)rand() << 32) | rand();
        } else if (mode == WRITE_ONES) {
            value = 1;
        } else {
            value = 0;
        }

        uint8_t *ptr = base + i * elem_size;

        // write based on width
        switch (elem_size) {
            case 1:
                *(uint8_t *)ptr = (uint8_t)value;
                break;
            case 2:
                *(uint16_t *)ptr = (uint16_t)value;
                break;
            case 4:
                *(uint32_t *)ptr = (uint32_t)value;
                break;
            case 8:
                *(int64_t *)ptr = (uint64_t)value;
                break;
        }
    }
}

void write_test_array_1D(UINTPTR base_addr,
                         uint32_t length,
                         uint32_t mode,
                         uint32_t width,
                         uint32_t stride)
{
    uint32_t i;
    uint32_t elem_size = width / 8;

    uint8_t *base = (uint8_t *)base_addr;

    for (i = 0; i < length; i++) {
        uint8_t *ptr = base + i * elem_size;

        switch (elem_size) {
            case 1: {
                int8_t value = (mode == WRITE_RAND) ? (int8_t)(rand() % 256 - 128)
                              : (mode == WRITE_ONES) ? -1
                              : 0;
                *(int8_t *)ptr = value;
                break;
            }
            case 2: {
                int16_t value = (mode == WRITE_RAND) ? (int16_t)(rand() % 65536 - 32768)
                               : (mode == WRITE_ONES) ? -1
                               : 0;
                *(int16_t *)ptr = value;
                break;
            }
            case 4: {
                int32_t value = (mode == WRITE_RAND) ? (int32_t)rand()
                               : (mode == WRITE_ONES) ? -1
                               : 0;
                *(int32_t *)ptr = value;
                break;
            }
            case 8: {
                int64_t value = 0;
                if (mode == WRITE_RAND) {
                    value = ((int64_t)(uint32_t)rand() << 32) | (uint32_t)rand();
                } else if (mode == WRITE_ONES) {
                    value = -1;
                }
                *(int64_t *)ptr = value;
                break;
            }
        }
    }
}

void write_test_array_1D_fp(UINTPTR base_addr,
                            uint32_t length,
                            uint32_t mode,
                            uint32_t width,
                            uint32_t stride)
{
    uint32_t i;
    uint32_t elem_size = width / 8;
    uint8_t *base = (uint8_t *)base_addr;

    (void)stride;  // currently unused

    for (i = 0; i < length; i++) {
        uint8_t *ptr = base + i * elem_size;

        switch (elem_size) {
            case 4: {
                float value;
                if (mode == WRITE_RAND) {
                    value = (float)rand() / (float)RAND_MAX;
                } else if (mode == WRITE_ONES) {
                    value = 1.0f;
                } else {
                    value = 0.0f;
                }
                *(float *)ptr = value;
                break;
            }

            case 8: {
                double value;
                if (mode == WRITE_RAND) {
                    value = (double)rand() / (double)RAND_MAX;
                } else if (mode == WRITE_ONES) {
                    value = 1.0;
                } else {
                    value = 0.0;
                }
                *(double *)ptr = value;
                break;
            }

            default:
                break;
        }
    }
}



void write_labeled_test_array_1D_fp_to_mem_and_txt(UINTPTR base_addr,
                                                   uint32_t length,
                                                   uint32_t mode,
                                                   uint32_t width,
                                                   uint32_t stride,
                                                   const char *file_stem,
                                                   char array_name)
{
    uint32_t i;
    uint32_t elem_size = width / 8;
    uint8_t *base = (uint8_t *)base_addr;
    char out_name[64];
    char line[64];
    u32 name_i = 0;
    int Status;
    FIL *fp = 0;
    UINT bytes_written;

    uint32_t upper;
uint32_t lower;

    if (file_stem == 0 || file_stem[0] == 0) {
        strcpy(out_name, "memdump.txt");
    } else {
        while (file_stem[name_i] != 0 &&
               file_stem[name_i] != '.' &&
               name_i < (sizeof(out_name) - sizeof(".txt") - 1U)) {
            out_name[name_i] = file_stem[name_i];
            name_i++;
        }
        out_name[name_i] = 0;

        if (name_i == 0U) {
            strcpy(out_name, "memdump.txt");
        } else {
            strcat(out_name, ".txt");
        }
    }

    Status = SD_Init();
    if (Status == XST_FAILURE) {
        xil_printf("SD card init failed\r\n");
        return;
    }

    fp = openFile(out_name, 'a');
    if (fp == 0) {
        xil_printf("File opening failed\r\n");
        SD_Eject();
        return;
    }

    line[0] = array_name;
    line[1] = ':';
    line[2] = '\r';
    line[3] = '\n';
    line[4] = 0;
    f_write(fp, line, strlen(line), &bytes_written);


    if ((elem_size != 4U) && (elem_size != 8U)) {
        xil_printf("Unsupported FP width: %lu\r\n", (unsigned long)width);
        return;
    }

    if (stride == 0U) {
        stride = elem_size;
    }

    for (i = 0; i < length; i++) {
        uint8_t *ptr = base + (i * stride);

        if (elem_size == 4U) {
            float value = 0.0f;

            if (mode == WRITE_RAND) {
                value = (float)rand() / (float)RAND_MAX;
            } else if (mode == WRITE_ONES) {
                value = 1.0f;
            }

            memcpy(ptr, &value, sizeof(value));

            //u32 hex;
            //memcpy(&hex, &value, sizeof(hex));
            //if (i < 100) {


              snprintf(line, sizeof(line), "0x%08lx\r\n", *(u32*)&value);
              f_write(fp, line, strlen(line), &bytes_written);
          //}

            
        } 
        
        else {
            double value = 0.0;
            if (mode == WRITE_RAND) {
                value = (double)rand() / (double)RAND_MAX;
            } else if (mode == WRITE_ONES) {
                value = 1.0;
            }
            memcpy(ptr, &value, sizeof(value));
        }
    }

    Xil_DCacheFlushRange((INTPTR)base_addr, (u32)(length * stride));

    

    closeFile(fp);
    SD_Eject();
}



UINTPTR PREV_ARR_END_ADDR;

#define IS_1D 1
#define IS_1D_U 2
#define IS_1D_FP 3

#define ARRAY_BUFFER_STRIDE 0x100ULL

uint32_t src_cnt;

void reset_array_allocator(void) {
  PREV_ARR_END_ADDR = BUFFER_BASE_ADDR;
  src_cnt = 0;
}

uint32_t calc_arr_end(uint32_t ew, uint32_t size, uint32_t type) {
  uint32_t space_needed;
  if (type == ( IS_1D || IS_1D_U|| IS_1D_FP)) { 
    space_needed = ew * size;
  }
  // future add more
  return space_needed;
}

#define SPECIAL_NONE              0
#define SPECIAL_SET_LAST_NEG_ONE  1
#define SPECIAL_SET_FIRST_ONE     2

void apply_special_rule(UINTPTR base_addr,
                        uint32_t size,
                        uint32_t ew,
                        uint32_t special_rule) {
  uint32_t elem_size = ew / 8;

  if (size == 0) {
    return;
  }

  switch (special_rule) {
    case SPECIAL_NONE:
      break;

    case SPECIAL_SET_LAST_NEG_ONE: {
      UINTPTR addr = base_addr + ((size - 1) * elem_size);

      if (ew == 32) {
        *(uint32_t *)addr = (uint32_t)-1;
      }

      break;
    }

    case SPECIAL_SET_FIRST_ONE: {
      UINTPTR addr = base_addr;

      if (ew == 32) {
        *(uint32_t *)addr = 1;
      }

      break;
    }

    default:
      xil_printf("ERROR: unknown special_rule\r\n");
      while (1) {}
  }
}

void setup_array(char src_id,
                 uint32_t valToWrite,
                 const char * ew_full,
                 uint32_t size,
                 uint32_t special_rule,
                 uint32_t array_type, char *testname, int print_to_file) {

    uint32_t ew = 0;
    //xil_printf("ew_full = %s\r\n", ew_full);
    if (ew_full != NULL) {
        if (strcmp(ew_full, "float") == 0) {
            ew = 32;
            //xil_printf("FLOAT");
        } else if (strcmp(ew_full, "double") == 0) {
            ew = 64;
        } else if (strcmp(ew_full, "uint8_t") == 0) {
            ew = 8;
        } else if (strcmp(ew_full, "uint16_t") == 0) {
            ew = 16;
        } else if (strcmp(ew_full, "uint32_t") == 0) {
            ew = 32;
        } else if (strcmp(ew_full, "uint64_t") == 0) {
            ew = 64;
        }
    }
  
  UINTPTR base_addr;
  uint32_t arr_len = calc_arr_end(ew, size, IS_1D);
  base_addr = PREV_ARR_END_ADDR;
  PREV_ARR_END_ADDR = base_addr + arr_len + ((UINTPTR)(src_cnt) * ARRAY_BUFFER_STRIDE);
  //  xil_printf("Writing to base addr");

  if (PREV_ARR_END_ADDR > ARR_MAX_ADDR) {
    xil_printf("ERROR: array overflow, end addr = 0x%08x, max addr = 0x%08x\r\n",
               (unsigned int)PREV_ARR_END_ADDR,
               (unsigned int)ARR_MAX_ADDR);
    while (1) {}
}

  UINTPTR src_reg = ARR_BASE_PTR_ADDR + (src_cnt * SRC_REG_STRIDE);
  write_read_pattern(src_reg, (uint32_t)(base_addr & 0xFFFFFFFF));


if ((array_type == IS_1D) && (ew_full != NULL)) {

    if (strcmp(ew_full, "float") == 0 || strcmp(ew_full, "double") == 0) {
        // floating point
        if(print_to_file){
          write_labeled_test_array_1D_fp_to_mem_and_txt(base_addr, size, valToWrite, ew, 0, testname, src_id);

        }
        else{
          write_test_array_1D_fp(base_addr, size, valToWrite, ew, 0);
        }
        apply_special_rule(base_addr, size, ew, special_rule);
    }
    else if (strcmp(ew_full, "uint8_t") == 0 ||
             strcmp(ew_full, "uint16_t") == 0 ||
             strcmp(ew_full, "uint32_t") == 0 ||
             strcmp(ew_full, "uint64_t") == 0) {
        // unsigned integer
        write_test_array_1D_u(base_addr, size, valToWrite, ew, 0);
        apply_special_rule(base_addr, size, ew, special_rule);
    }
    else if (strcmp(ew_full, "int8_t") == 0 ||
             strcmp(ew_full, "int16_t") == 0 ||
             strcmp(ew_full, "int32_t") == 0 ||
             strcmp(ew_full, "int64_t") == 0 ||
             strcmp(ew_full, "int") == 0) {
        // signed integer
        write_test_array_1D(base_addr, size, valToWrite, ew, 0);
        apply_special_rule(base_addr, size, ew, special_rule);
    }
}

  src_cnt++;
}

void setup_1D(char src_id, uint32_t valToWrite, const char * ew_full, uint32_t size,
              uint32_t special_rule, char *testname, int print_to_file) {
  setup_array(src_id, valToWrite, ew_full, size, special_rule, IS_1D, testname, print_to_file);
}



void setup(const char *in_file) {
  bin_file_to_dram(in_file, (UINTPTR)0x480000000ULL, 0);
  clearTriggers((UINTPTR)0x49EEC0000ULL);
}











#define BENCHMARK_RES_ADDR 0x49FFF0030ULL


uint32_t runTest(uint32_t test_num, int print_to_file) {
  // this to force cheshire to wait between runs
  // so the previous run's results can be

  if (test_num == 1) {
    writeTrigger(BOOT_ADDR);
  }
  writeTrigger(START_ADDR);

  while (*(volatile uint32_t *)INIT_ADDR != 0xBABEBABE) {
  } // wait for init info
  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  char *ew = (char *)EW_ADDR;
  uint32_t arrLen = *(volatile uint32_t *)LEN_ADDR;
  char *test_name = (char *)NAME_ADDR;
  //xil_printf("ew = %s\r\n", ew);
  //xil_printf("name = %s\r\n", test_name);
  full_array_setup(test_name, ew, arrLen, 0, print_to_file);
  //uint64_t arm_result = check_s311_float(arrLen); //FIX, STOPS cheshire working 
  ////////////////////////////////////////////////////////////////
  // shared, don't touch
  ////////////////////////////////////////////////////////////////
  writeTrigger(ARR_ADDR); // tell RISC-V done array init
  uint32_t finalRun = checkELFDone((UINTPTR)DONE_ADDR); // check cheshire done test
  
  
  dram_print_benchmark_result(
      RESULTS_ADDR,
      test_num); // test_num is just for printing which test we are on
  write_result(RESULTS_ADDR, arrLen); // writes result to file
  //uint32_t cheshire_result = *(volatile uint32_t *)BENCHMARK_RES_ADDR; 
  //xil_printf("cheshire_result = 0x%08X\n", cheshire_result);

  
  finalRun = checkELFDone((UINTPTR)DONE_ADDR); 
   clearTriggers((UINTPTR)0x49EEC0000ULL);
   clearTriggers((UINTPTR)RESULTS_ADDR);
   clearTriggers((UINTPTR)0x49EEE0000ULL);
  return finalRun;
}