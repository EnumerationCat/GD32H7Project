#include "boot_cmd.h"

/* 返回 1 告诉 shell() 退出主循环，由 bootloader 继续跳 App */
static int boot_f(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return 1;
}

struct command boot_cmd = {
	"boot",

	"exit shell and jump to App\r\n",

	"Usage: \r\n"
	"       boot, exit shell and jump to App\r\n",

	boot_f,
};

struct command jump_cmd = {
	"jump",

	"alias of boot\r\n",

	"Usage: \r\n"
	"       jump, alias of boot\r\n",

	boot_f,
};

struct command exit_cmd = {
	"exit",

	"alias of boot\r\n",

	"Usage: \r\n"
	"       exit, alias of boot\r\n",

	boot_f,
};
