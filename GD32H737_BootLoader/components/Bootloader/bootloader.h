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

/* 调试串口（USART0 / PA9 TX / PA10 RX / 115200-8-N-1）
   发送轮询，接收由 RX 中断写入 ring buffer，此处非阻塞取字节 */
void boot_putc(uint8_t c);
void boot_puts(const char *s);
int  boot_getc(void);                                  /* 无数据返回 -1 */
int  boot_getc_timeout(uint32_t timeout_ms);           /* 超时返回 -1，否则 0~255 */

/* 无格式化输出的整数打印 */
void boot_putdec(uint32_t v);
void boot_puthex32(uint32_t v);
