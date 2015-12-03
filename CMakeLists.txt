cmake_minimum_required(VERSION 3.3)
project(GeneralC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
set(SOURCE_FILES HW12/my_malloc.c HW12/my_sbrk.c HW12/test.c)
add_executable(GeneralC ${SOURCE_FILES})
set_property(TARGET GeneralC PROPERTY C_STANDARD 99)