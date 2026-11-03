#pragma once

#include <stdint.h>

/* Flash 分区：前 60K 给 bootloader，最后 4K（0x0800F000）为启动配置扇区。
   App 分 A/B 两槽，2MB 边界；每槽起始 64 字节为 uImage 头，App 本体在其后。 */
#define BOOT_CFG_ADDR       0x0800F000U

#define APP_SLOT_A_ADDR     0x08010000U
#define APP_SLOT_A_END      0x08200000U
#define APP_SLOT_B_ADDR     0x08200000U
#define APP_SLOT_B_END      0x083C0000U

#define APP_SLOT_A_SIZE     (APP_SLOT_A_END - APP_SLOT_A_ADDR)
#define APP_SLOT_B_SIZE     (APP_SLOT_B_END - APP_SLOT_B_ADDR)
#define FLASH_END_ADDR      APP_SLOT_B_END

/* App 栈顶所在 RAM 范围（AXI SRAM 512K，共享 RAM 已全部划为 TCM） */
#define APP_SRAM_START      0x24000000U
#define APP_SRAM_END        0x24080000U

#define IMAGE_MAGIC         0x27051956U
#define IMAGE_HEADER_SIZE   64U
#define IMAGE_NAME_LEN      32U

typedef unsigned int __be32;

typedef struct image_header {
	__be32      ih_magic;   /* Image Header Magic Number */
	__be32      ih_hcrc;    /* Image Header CRC Checksum */
	__be32      ih_time;    /* Image Creation Timestamp */
	__be32      ih_size;    /* Image Data Size          */
	__be32      ih_load;    /* Data Load Address         */
	__be32      ih_ep;      /* Entry Point Address       */
	__be32      ih_dcrc;    /* Image Data CRC Checksum   */
	uint8_t     ih_os;      /* Operating System          */
	uint8_t     ih_arch;    /* CPU architecture          */
	uint8_t     ih_type;    /* Image Type                */
	uint8_t     ih_comp;    /* Compression Type          */
	uint8_t     ih_name[IMAGE_NAME_LEN]; /* Image Name    */
} image_header_t;

_Static_assert(sizeof(image_header_t) == IMAGE_HEADER_SIZE, "image_header_t must be 64 bytes");

typedef struct boot_image_info {
	uint32_t image_base;    /* uImage 头地址 */
	uint32_t load;          /* ih_load，向量表地址 */
	uint32_t ep;            /* ih_ep，入口地址 */
	uint32_t size;          /* ih_size，含头部之后全部 payload */
	uint32_t dcrc;          /* ih_dcrc */
} boot_image_info_t;

/* 校验 image_base 处的 uImage 头（可选头/数据 CRC）；合法返回 1，
   并通过 info 返回加载地址等信息。
   src_len:   image_base 处可安全读取的字节数
   slot_*:    镜像最终写入/运行的目标槽，load/ep/size 必须落在其中 */
int bootloader_image_parse(uint32_t image_base, uint32_t src_len,
                           uint32_t slot_base, uint32_t slot_end,
                           int verify_crc, boot_image_info_t *info);

/* 把控制权交给 load_addr 处的 App，不返回 */
void bootloader_jump_to_app(uint32_t load_addr);

/* shell 命令设置本次跳转目标；0 表示未指定 */
void boot_set_target(uint32_t load_addr);
uint32_t boot_get_target(void);

/* 调试串口（USART0 / PA9 TX / PA10 RX / 115200-8-N-1）
   发送轮询，接收由 RX 中断写入 ring buffer，此处非阻塞取字节 */
void boot_putc(uint8_t c);
void boot_puts(const char *s);
int  boot_getc(void);                                  /* 无数据返回 -1 */
int  boot_getc_timeout(uint32_t timeout_ms);           /* 超时返回 -1，否则 0~255 */

/* 等 TX 移位寄存器清空，确保复位/跳转前最后几个字节已发出 */
void boot_uart_flush(void);

/* 丢弃串口接收侧所有残留字节（ring buffer + 硬件 FIFO）。
   ZMODEM 失败/结束后调用，避免半包被 shell 当成命令回显执行。 */
void boot_uart_rx_flush(void);

/* 无格式化输出的整数打印 */
void boot_putdec(uint32_t v);
void boot_puthex32(uint32_t v);
