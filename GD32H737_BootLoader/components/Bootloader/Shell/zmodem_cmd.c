#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "zmodem.h"
#include "command.h"
#include "file.h"
#include "bootloader.h"

static int rz(int argc, char **argv)
{
	size_t received;

	(void)argc;
	(void)argv;

	boot_puts("\r\n[rz] waiting for ZMODEM sender (send: sz -b file)...\r\n");
	boot_puts("[rz] RAM buffer: ");
	boot_puthex32((uint32_t)(uintptr_t)zmodem_file_buffer());
	boot_puts(" capacity: ");
	boot_putdec((uint32_t)zmodem_file_capacity());
	boot_puts(" bytes\r\n");

	received = zmodem_receive(NULL, /* 文件名由发送端决定，这里只关心内容 */
				  NULL, /* 不限制文件 */
				  NULL, /* 无进度回调 */
				  NULL, /* 无完成回调 */
				  0,    /* 不限速 */
				  RZSZ_FLAGS_NONE);

	boot_puts("[rz] received ");
	boot_putdec((uint32_t)received);
	boot_puts(" bytes at ");
	boot_puthex32((uint32_t)(uintptr_t)zmodem_file_buffer());
	boot_puts("\r\n");

	return 0;
}

struct command rz_cmd = {
	"rz",

	"receive a file into RAM via ZMODEM\r\n",

	"Usage: \r\n"
	"       rz, receive a file into RAM via ZMODEM\r\n",

	rz,
};
