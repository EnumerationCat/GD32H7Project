#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "zmodem.h"
#include "command.h"
#include "file.h"
#include "shell.h"
#include "bootloader.h"
#include "boot_flash.h"

static int rz(int argc, char **argv)
{
	size_t received;
	unsigned char *buf;
	boot_image_info_t info;

	(void)argc;
	(void)argv;

	shell_putstr("\r\n[rz] waiting for ZMODEM sender (send: sz -b u-boot_app.img)...\r\n");
	shell_putstr("[rz] RAM buffer: ");
	shell_puthex32((uint32_t)(uintptr_t)zmodem_file_buffer());
	shell_putstr(" capacity: ");
	shell_putdec((uint32_t)zmodem_file_capacity());
	shell_putstr(" bytes\r\n");

	received = zmodem_receive(NULL, /* 文件名由发送端决定，这里只关心内容 */
				  NULL, /* 不限制文件 */
				  NULL, /* 无进度回调 */
				  NULL, /* 无完成回调 */
				  0,    /* 不限速 */
				  RZSZ_FLAGS_NONE);

	shell_putstr("[rz] received ");
	shell_putdec((uint32_t)received);
	shell_putstr(" bytes\r\n");

	buf = zmodem_file_buffer();

	if(0U == received) {
		shell_putstr("[rz] no data, abort\r\n");
		return 0;
	}
	if(received > APP_SLOT_B_SIZE) {
		shell_putstr("[rz] image larger than slot B, abort\r\n");
		return 0;
	}

	/* 校验 uImage 头与数据 CRC，并要求 ih_load 落在 B 槽内 */
	if(!bootloader_image_parse((uint32_t)(uintptr_t)buf, (uint32_t)received,
				   APP_SLOT_B_ADDR, APP_SLOT_B_END, 1, &info)) {
		shell_putstr("[rz] invalid uImage (magic/CRC/load must be in slot B)\r\n");
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
	shell_putstr("\r\n[rz] writing slot B ...\r\n");

	if(!boot_flash_write_image(APP_SLOT_B_ADDR, buf, (uint32_t)received)) {
		shell_putstr("[rz] flash erase/write/verify FAILED\r\n");
		return 0;
	}
	shell_putstr("[rz] flash verify OK\r\n");

	if(!boot_cfg_set_active_slot(1)) {
		shell_putstr("[rz] boot config write FAILED\r\n");
		return 0;
	}

	shell_putstr("[rz] OK, next boot slot = B, load=");
	shell_puthex32(info.load);
	shell_putstr("\r\n");

	return 0;
}

struct command rz_cmd = {
	"rz",

	"receive uImage via ZMODEM and write to app slot B\r\n",

	"Usage: \r\n"
	"       rz, receive a uImage (with 64-byte boot header) into slot B\r\n"
	"           and mark slot B for next boot\r\n",

	rz,
};
