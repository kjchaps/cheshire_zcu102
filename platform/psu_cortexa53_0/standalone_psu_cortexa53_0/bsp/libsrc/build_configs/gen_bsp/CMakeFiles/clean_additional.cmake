# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/diskio.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/ff.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/ffconf.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/sleep.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/xilffs.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/xilffs_config.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/xiltimer.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/include/xtimer_config.h"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/lib/libxilffs.a"
  "/home3/shared/chapman44/cheshire/platform/psu_cortexa53_0/standalone_psu_cortexa53_0/bsp/lib/libxiltimer.a"
  )
endif()
