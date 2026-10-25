cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/cortex_m7.cmake")
message("[ROOT] Build Type: ${CMAKE_BUILD_TYPE}")
message("[ROOT] Toolchain File: ${CMAKE_TOOLCHAIN_FILE}")

project(app LANGUAGES C CXX ASM)

# 生成 .s 反汇编清单需要 Python
find_package(Python3 COMPONENTS Interpreter REQUIRED)
get_filename_component(ARM_TOOLCHAIN_BIN "${CMAKE_OBJDUMP}" DIRECTORY)
set(ARM_NM "${ARM_TOOLCHAIN_BIN}/arm-none-eabi-nm.exe")

# 加头工具：mkimage 依赖的 msys-2.0.dll 已随 tools/ 提供
find_program(MKIMAGE_EXECUTABLE mkimage HINTS "${CMAKE_SOURCE_DIR}/tools" NO_DEFAULT_PATH)
if(NOT MKIMAGE_EXECUTABLE)
    message(FATAL_ERROR "mkimage not found in ${CMAKE_SOURCE_DIR}/tools")
endif()

# ---------------- App：链接在 0x08010400（0x08010000 起 64 字节 uImage 头 + 填充到 1024 对齐） ----------------
add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# ---------------- Bootloader：链接在 0x08000000（前 64K） ----------------
add_executable(bootloader)
set_target_properties(bootloader PROPERTIES SUFFIX ".elf")

# 各子目录据此把共享源码同时加到 App 与 Bootloader
set(FIRMWARE_TARGETS "${PROJECT_NAME};bootloader")

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

# App：0x08010000 起 64 字节是 uImage 头，向量表在 0x08010400
# （186 项 -> 1024 字节对齐，只做 128/512 对齐会让外部中断取向量落错地址）
target_compile_definitions(${PROJECT_NAME} PRIVATE VECT_TAB_OFFSET=0x10400)

# Bootloader：向量表基址回 0x08000000，并需要 gd32h7xx_conf.h
target_compile_definitions(bootloader PRIVATE VECT_TAB_OFFSET=0x0)
target_include_directories(bootloader PRIVATE "${CMAKE_SOURCE_DIR}/src/app/config")

#app（仅 App）
add_subdirectory(src/app)

# driver（App + Bootloader）
add_subdirectory(src/driver)

# components（BSP/Middlewares 仅 App；Bootloader 仅 bootloader）
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
target_link_options(bootloader PRIVATE
    -T${CMAKE_SOURCE_DIR}/bootloader.ld
    -Wl,-Map=bootloader.map
)

foreach(t ${FIRMWARE_TARGETS})
    add_custom_command(TARGET ${t} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${t}.elf ${t}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${t}.elf ${t}.bin
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/cmake/gen_disasm.py"
            --objdump "${CMAKE_OBJDUMP}"
            --nm "${ARM_NM}"
            --elf "${t}.elf"
            --out "${t}.s"
        COMMENT "Generating ${t}.s"
        VERBATIM
    )
endforeach()

# App 加头产物：u-boot_app.img（烧到 0x08010000）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/cmake/gen_uimage.py"
        --mkimage "${MKIMAGE_EXECUTABLE}"
        --nm "${ARM_NM}"
        --elf "${PROJECT_NAME}.elf"
        --bin "${PROJECT_NAME}.bin"
        --base 0x08010000
        --load 0x08010400
        --name "GD32H737-APP"
        --out "u-boot_app.img"
    COMMENT "Generating u-boot_app.img"
    VERBATIM
)
