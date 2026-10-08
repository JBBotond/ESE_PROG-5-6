# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "debug")
  file(REMOVE_RECURSE
  "clean_files-NOTFOUND"
  "mcxn947-bmp280_cm33_core0.bin"
  )
endif()
