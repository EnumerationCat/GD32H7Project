#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "zmodem.h"
#include "command.h"
#include "file.h"
#include "shell.h"
#include "bootloader.h"
#include "boot_flash.h"
#include "boot_display.h"

static int rz_slot_valid(int slot)
{
	uint32_t base = slot ? APP_SLOT_B_ADDR : APP_SLOT_A_ADDR;
	uint32_t end  = slot ? APP_SLOT_B_END  : APP_SLOT_A_END;
	uint32_t size = slot ? APP_SLOT_B_SIZE : APP_SLOT_A_SIZE;

	return bootloader_image_parse(base, size, base, end, 1, NULL);
}

/* 当前启动槽有效就写另一槽（A/B 轮换）；当前槽无效则写回该槽修复。
   返回 0 = A 槽，1 = B 槽。 */
static int rz_pick_target_slot(void)
{
	int active = boot_cfg_get_active_slot();
	int other  = active ? 0 : 1;

	if(rz_slot_valid(active)) {
		return other;
	}
	if(rz_slot_valid(other)) {
		return active;
	}
	return other;
}

/* 无 printf 的十进制转换：bootloader 不链接格式化库 */
static uint32_t rz_dec(char *p, uint32_t v)
{
	char tmp[10];
	uint32_t n = 0U;
	uint32_t i;

	do {
		tmp[n++] = (char)('0' + (v % 10U));
		v /= 10U;
	} while(0U != v);

	for(i = 0U; i < n; i++) {
		p[i] = tmp[n - 1U - i];
	}
	return n;
}

/* ZMODEM 收包间隙回调：只在 LCD 上刷新一行进度，不往串口写 */
static bool rz_tick(const char *fname, long bytes_sent, long bytes_total,
		    long last_bps, int min_left, int sec_left)
{
	char line[64];
	uint32_t n = 0U;
	uint32_t recv = (uint32_t)bytes_sent;
	uint32_t total = (uint32_t)bytes_total;
	uint32_t pct = 0U;

	(void)fname;
	(void)last_bps;
	(void)min_left;
	(void)sec_left;

	if(total > 0U) {
		pct = (uint32_t)(((unsigned long)recv * 100UL) / (unsigned long)total);
		if(pct > 100U) {
			pct = 100U;
		}
	}

	memcpy(line, "[rz] receiving ", 15U);
	n = 15U;
	n += rz_dec(&line[n], recv);
	line[n++] = ' ';
	line[n++] = '/';
	line[n++] = ' ';
	n += rz_dec(&line[n], total);
	line[n++] = ' ';
	line[n++] = '(';
	n += rz_dec(&line[n], pct);
	line[n++] = '%';
	line[n++] = ')';
	line[n] = '\0';

	boot_display_progress(line);
	return true;
}

static int rz(int argc, char **argv)
{
	size_t received;
	unsigned char *buf;
	boot_image_info_t info;
	int target;
	uint32_t target_base, target_end, target_size;

	(void)argc;
	(void)argv;

	target      = rz_pick_target_slot();
	target_base = target ? APP_SLOT_B_ADDR : APP_SLOT_A_ADDR;
	target_end  = target ? APP_SLOT_B_END  : APP_SLOT_A_END;
	target_size = target ? APP_SLOT_B_SIZE : APP_SLOT_A_SIZE;

	shell_putstr("\r\n[rz] waiting for ZMODEM sender (send: sz -b u-boot_app");
	shell_putstr(target ? "B" : "A");
	shell_putstr(".img)...\r\n");
	shell_putstr("[rz] RAM buffer: ");
	shell_puthex32((uint32_t)(uintptr_t)zmodem_file_buffer());
	shell_putstr(" capacity: ");
	shell_putdec((uint32_t)zmodem_file_capacity());
	shell_putstr(" bytes\r\n");

	/* 先把提示刷到屏幕：下面 zmodem_receive 会阻塞到传输结束 */
	boot_display_poll();

	/* 提前建好进度行，传输中只需覆盖最后一行，避免整屏重画拖慢收包 */
	boot_display_progress("[rz] receiving ...");

	/* 丢掉进入 ZMODEM 前 RX 侧残留（命令行的换行、终端应答、上次会话尾部），
	   避免启动阶段把 ZRQINIT/ZFILE 的字节对齐打乱 */
	boot_uart_rx_flush();

	/* 传输过程中的协议字节不要镜像到屏幕日志 */
	boot_display_suspend(1);
	received = zmodem_receive(NULL, /* 文件名由发送端决定，这里只关心内容 */
				  NULL, /* 不限制文件 */
				  rz_tick, /* 收包间隙更新屏幕进度 */
				  NULL, /* 无完成回调 */
				  0,    /* 不限速 */
				  RZSZ_FLAGS_NONE);
	boot_display_suspend(0);

	/* 进度行收尾，后续日志另起一行 */
	boot_display_progress_end();

	/* 失败时发送端可能仍在补发，接收缓冲里会残留半包二进制；
	   不清掉的话回到 shell 会被当成命令回显执行 */
	boot_uart_rx_flush();

	shell_putstr("[rz] received ");
	shell_putdec((uint32_t)received);
	shell_putstr(" bytes\r\n");

	buf = zmodem_file_buffer();

	shell_putstr("[rz] target slot ");
	shell_putstr(target ? "B" : "A");
	shell_putstr(" image=");
	shell_puthex32(target_base);
	shell_putstr("\r\n");

	if(0U == received) {
		shell_putstr("[rz] transfer failed or no data, abort\r\n");
		return 0;
	}
	if(received > target_size) {
		shell_putstr("[rz] image larger than target slot, abort\r\n");
		return 0;
	}

	/* 校验 uImage 头与数据 CRC，并要求 ih_load 落在目标槽内 */
	if(!bootloader_image_parse((uint32_t)(uintptr_t)buf, (uint32_t)received,
				   target_base, target_end, 1, &info)) {
		shell_putstr("[rz] invalid uImage (magic/CRC/load must be in target slot)\r\n");
		return 0;
	}

	if((uint32_t)received != (IMAGE_HEADER_SIZE + info.size)) {
		shell_putstr("[rz] image truncated or has trailing data, abort\r\n");
		return 0;
	}

	shell_putstr("[rz] image load=");
	shell_puthex32(info.load);
	shell_putstr(" ep=");
	shell_puthex32(info.ep);
	shell_putstr(" size=");
	shell_putdec(info.size);
	shell_putstr("\r\n[rz] writing slot ");
	shell_putstr(target ? "B" : "A");
	shell_putstr(" ...\r\n");

	/* 擦除/写入/校验可能耗时较久，先让提示上屏 */
	boot_display_poll();

	if(!boot_flash_write_image(target_base, buf, (uint32_t)received)) {
		shell_putstr("[rz] flash erase/write/verify FAILED\r\n");
		return 0;
	}
	shell_putstr("[rz] flash verify OK\r\n");
	boot_display_poll();

	if(!boot_cfg_set_active_slot(target)) {
		shell_putstr("[rz] boot config write FAILED\r\n");
		return 0;
	}

	shell_putstr("[rz] OK, next boot slot = ");
	shell_putstr(target ? "B" : "A");
	shell_putstr(", load=");
	shell_puthex32(info.load);
	shell_putstr("\r\n");

	return 0;
}

struct command rz_cmd = {
	"rz",

	"receive uImage via ZMODEM and write to the inactive app slot\r\n",

	"Usage: \r\n"
	"       rz, receive a uImage (with 64-byte boot header) into the\r\n"
	"           inactive slot and mark it for next boot\r\n",

	rz,
};
