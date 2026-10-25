#pragma once

#include <stdint.h>

/* Flash 分区：前 64K 给本 bootloader；0x08010000 起 64 字节为 uImage 头，App 本体在其后 */
#define BOOT_SIZE           (64U * 1024U)
#define APP_BASE_ADDR       (0x08000000U + BOOT_SIZE)            /* 0x08010000，uImage 头地址 */
#define FLASH_END_ADDR      (0x08000000U + (3840U * 1024U))

/* App 栈顶所在 RAM 范围（AXI SRAM 512K，共享 RAM 已全部划为 TCM） */
#define APP_SRAM_START      0x24000000U
#define APP_SRAM_END        0x24080000U

/* 校验 image_base 处的 uImage 头与向量表；合法返回 1，并通过 app_base 返回 App 向量表地址 */
int  bootloader_app_is_valid(uint32_t image_base, uint32_t *app_base);

/* 把控制权交给 app_base 处的 App，不返回 */
void bootloader_jump_to_app(uint32_t app_base);
