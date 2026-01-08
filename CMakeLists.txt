cmake_minimum_required(VERSION 3.16)

project(RTNS
    VERSION 1.0.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(RTNS_USE_PLATFORM_STUBS "Use host stub implementations for platform hooks" OFF)
set(RTNS_PLATFORM "NONE" CACHE STRING "Platform implementation: NONE, FREERTOS, BAREMETAL")
option(RTNS_ENABLE_ASAN "Enable AddressSanitizer for debug builds" OFF)

if(RTNS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

# Sources
set(RTNS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_ipv6.c
)

set(RTNS_STUB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/rtnet_platform_stubs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/rtnet_host_api_stubs.c
)

set(RTNS_PLATFORM_SOURCES "")
if(RTNS_PLATFORM STREQUAL "FREERTOS")
    list(APPEND RTNS_PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/rtnet_platform_freertos.c)
elseif(RTNS_PLATFORM STREQUAL "BAREMETAL")
    list(APPEND RTNS_PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/platforms/rtnet_platform_baremetal.c)
endif()

if(RTNS_USE_PLATFORM_STUBS)
    list(APPEND RTNS_SOURCES ${RTNS_STUB_SOURCES})
elseif(NOT RTNS_PLATFORM STREQUAL "NONE")
    list(APPEND RTNS_SOURCES ${RTNS_PLATFORM_SOURCES})
endif()

add_library(rtns STATIC ${RTNS_SOURCES})
target_include_directories(rtns PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Examples (desktop/host)
option(RTNS_BUILD_EXAMPLES "Build desktop examples" ON)

# Warnings
if(MSVC)
    target_compile_options(rtns PRIVATE /W4)
else()
    target_compile_options(rtns PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Test executable (uses the library and optionally platform stubs)
if(RTNS_USE_PLATFORM_STUBS)
    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtnet_test_suite.c
        ${RTNS_STUB_SOURCES}
    )

    add_executable(rtns_tests ${TEST_SOURCES})
    target_link_libraries(rtns_tests PRIVATE rtns)
    target_include_directories(rtns_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    if(MSVC)
        target_compile_options(rtns_tests PRIVATE /W4)
    else()
        target_compile_options(rtns_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
else()
    message(STATUS "RTNS_USE_PLATFORM_STUBS=OFF: skipping host test target")
endif()

if(RTNS_BUILD_EXAMPLES AND RTNS_USE_PLATFORM_STUBS)
    add_executable(rtns_desktop_demo
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/desktop_demo/main.c
        ${RTNS_STUB_SOURCES}
    )
    target_link_libraries(rtns_desktop_demo PRIVATE rtns)
    target_include_directories(rtns_desktop_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    if(MSVC)
        target_compile_options(rtns_desktop_demo PRIVATE /W4)
    else()
        target_compile_options(rtns_desktop_demo PRIVATE -Wall -Wextra -Wpedantic)
    endif()
elseif(RTNS_BUILD_EXAMPLES AND NOT RTNS_USE_PLATFORM_STUBS)
    message(STATUS "RTNS_USE_PLATFORM_STUBS=OFF: skipping desktop_demo example")
endif()
