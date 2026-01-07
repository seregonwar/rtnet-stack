cmake_minimum_required(VERSION 3.16)

project(RTNS
    VERSION 1.0.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(RTNS_USE_PLATFORM_STUBS "Use host stub implementations for platform hooks" ON)
option(RTNS_ENABLE_ASAN "Enable AddressSanitizer for debug builds" OFF)

if(RTNS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

# Sources
set(RTNS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_ipv6.c
)

if(RTNS_USE_PLATFORM_STUBS)
    list(APPEND RTNS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_platform_stubs.c)
endif()

add_library(rtns STATIC ${RTNS_SOURCES})
target_include_directories(rtns PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Test executable (uses the library and optionally platform stubs)
set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_test_suite.c
)

if(RTNS_USE_PLATFORM_STUBS)
    list(APPEND TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_platform_stubs.c)
endif()

add_executable(rtns_tests ${TEST_SOURCES})
target_link_libraries(rtns_tests PRIVATE rtns)
target_include_directories(rtns_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Warnings
if(MSVC)
    target_compile_options(rtns PRIVATE /W4)
    target_compile_options(rtns_tests PRIVATE /W4)
else()
    target_compile_options(rtns PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(rtns_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
