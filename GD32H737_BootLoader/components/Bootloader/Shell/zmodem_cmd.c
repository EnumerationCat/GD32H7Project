#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "zmodem.h"
#include "command.h"
#include "file.h"
#include "shell.h"

static int rz(int argc, char **argv)
{
	size_t received;

	(void)argc;
	(void)argv;

	shell_putstr("\r\n[rz] waiting for ZMODEM sender (send: sz -b file)...\r\n");
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
	shell_putstr(" bytes at ");
	shell_puthex32((uint32_t)(uintptr_t)zmodem_file_buffer());
	shell_putstr("\r\n");

	return 0;
}

struct command rz_cmd = {
	"rz",

	"receive a file into RAM via ZMODEM\r\n",

	"Usage: \r\n"
	"       rz, receive a file into RAM via ZMODEM\r\n",

	rz,
};
