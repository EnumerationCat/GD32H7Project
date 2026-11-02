#pragma once

#include <stdint.h>

#define BOOT_FLASH_SECTOR_SIZE  0x1000U

/* 擦除覆盖 [addr, addr+len) 的 4KB 扇区；成功返回 1 */
int boot_flash_erase(uint32_t addr, uint32_t len);

/* 32bit 字编程，末尾不足 4 字节按 0xFF 补齐；addr 需 4 字节对齐；成功返回 1 */
int boot_flash_write(uint32_t addr, const void *data, uint32_t len);

/* 擦除目标槽并写入完整 uImage（含 64 字节头），最后读回比对；成功返回 1。
   先写 payload、最后写头，防止掉电留下“头合法但数据不全”的镜像。 */
int boot_flash_write_image(uint32_t slot_base, const uint8_t *data, uint32_t len);

/* 配置扇区：0 = A 槽，1 = B 槽 */
int boot_cfg_get_active_slot(void);
int boot_cfg_set_active_slot(int slot);
