cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/cortex_m7.cmake")
message("[ROOT] Build Type: ${CMAKE_BUILD_TYPE}")
message("[ROOT] Toolchain File: ${CMAKE_TOOLCHAIN_FILE}")

project(DEMO LANGUAGES C CXX ASM)

# ---------------- App：链接在 0x08010000 ----------------
add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# ---------------- Bootloader：链接在 0x08000000（前 64K） ----------------
add_executable(BOOT)
set_target_properties(BOOT PROPERTIES SUFFIX ".elf")

# 各子目录据此把共享源码同时加到 App 与 Bootloader
set(FIRMWARE_TARGETS "${PROJECT_NAME};BOOT")

# cmsis-core / cmsis-compiler：两个目标共用
foreach(t ${FIRMWARE_TARGETS})
    target_include_directories(${t} PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/arm/cmsis-core/Include"
        "${CMAKE_SOURCE_DIR}/third_party/arm/cmsis-compiler/include"
        "${CMAKE_SOURCE_DIR}/third_party/arm/cmsis-compiler/include/gcc"
    )
    target_sources(${t} PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/arm/cmsis-compiler/source/gcc/retarget_syscalls.c"
    )
endforeach()

# Bootloader：向量表基址回 0x08000000，并需要 gd32h7xx_conf.h
target_compile_definitions(BOOT PRIVATE VECT_TAB_OFFSET=0x0)
target_include_directories(BOOT PRIVATE "${CMAKE_SOURCE_DIR}/src/app/config")

#app（仅 App）
add_subdirectory(src/app)

# driver（App + Bootloader）
add_subdirectory(src/driver)

# components（BSP/Middlewares 仅 App；Bootloader 仅 BOOT）
add_subdirectory(components/Bootloader)
add_subdirectory(components/BSP)
add_subdirectory(components/Middlewares)

# GD32H7xx_Driver（App + Bootloader）
add_subdirectory(third_party/gd/GD32H7xx_Driver)

# ---------------- 链接脚本与产物 ----------------
target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/gd32h7xx_flash.ld
    -Wl,-Map=${PROJECT_NAME}.map
)
target_link_options(BOOT PRIVATE
    -T${CMAKE_SOURCE_DIR}/bootloader.ld
    -Wl,-Map=BOOT.map
)

foreach(t ${FIRMWARE_TARGETS})
    add_custom_command(TARGET ${t} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${t}.elf ${t}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${t}.elf ${t}.bin
    )
endforeach()
