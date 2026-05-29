cmake_minimum_required(VERSION 3.18)
project(raplt_avd_test C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAPLT_ROOT ${CMAKE_SOURCE_DIR}/../../../../../..)

# RaPLT core objects (static lib)
add_library(raplt_static STATIC
    ${RAPLT_ROOT}/src/raplt_core.c
    ${RAPLT_ROOT}/src/raplt_elf.c
    ${RAPLT_ROOT}/src/raplt_hash.c
    ${RAPLT_ROOT}/src/raplt_signal.c
    ${RAPLT_ROOT}/src/raplt_util.c
    ${RAPLT_ROOT}/src/raplt_recon.c
    ${RAPLT_ROOT}/src/raplt_dlopen.c
    ${RAPLT_ROOT}/src/raplt_cfi.c
)

target_include_directories(raplt_static PUBLIC
    ${RAPLT_ROOT}/include
)
target_compile_options(raplt_static PRIVATE
    -Wall -Wextra -fvisibility=hidden
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-unused-but-set-variable
)

# Test target library (gets hooked)
add_library(test_target SHARED test_target.c)

# JNI bridge
add_library(test_jni SHARED test_jni.c)
target_link_libraries(test_jni raplt_static log dl)
target_include_directories(test_jni PRIVATE ${RAPLT_ROOT}/include)
