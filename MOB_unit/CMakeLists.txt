cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mob_unit)
target_sources(app PRIVATE ${app_sources})
set(NRF_SUPPORTED_BUILD_TYPES "ZDebug;ZRelease")
FILE(GLOB app_sources src/*.c)
target_include_directories(app PRIVATE include)
zephyr_include_directories(src)

