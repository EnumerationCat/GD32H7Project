#include <stddef.h>
#include <string.h>

#include "app_shell_cmd.h"
#include "app_shell.h"
#include "gd32h7xx.h"

#define APP_SHELL_ARG_MAX    8

static int app_shell_help_f(int argc, char **argv);
static int app_shell_clear_f(int argc, char **argv);
static int app_shell_reboot_f(int argc, char **argv);

static struct app_shell_command help_cmd = {
	"help",

	"list all commands\r\n",

	"Usage: \r\n"
	"       help, list all commands\r\n"
	"       help command, show usage of 'command'\r\n",

	app_shell_help_f,
};

static struct app_shell_command clear_cmd = {
	"clear",

	"clear the shell screen\r\n",

	"Usage: \r\n"
	"       clear, clear the shell screen\r\n",

	app_shell_clear_f,
};

static struct app_shell_command reboot_cmd = {
	"reboot",

	"reset the MCU\r\n",

	"Usage: \r\n"
	"       reboot, reset the MCU\r\n",

	app_shell_reboot_f,
};

static struct app_shell_command *g_commands[] = {
	&help_cmd,
	&clear_cmd,
	&reboot_cmd,
	NULL
};

struct app_shell_command **app_shell_get_cmds(void)
{
	return g_commands;
}

struct app_shell_command *app_shell_find_cmd(const char *name)
{
	int i = 0;

	while(g_commands[i])
	{
		if(0 == strcmp(g_commands[i]->name, name))
		{
			return g_commands[i];
		}
		i++;
	}

	return NULL;
}

static int app_shell_help_f(int argc, char **argv)
{
	int i = 0;
	struct app_shell_command **cmds = app_shell_get_cmds();

	if(1 == argc)
	{
		while(cmds[i])
		{
			app_shell_putstr(cmds[i]->name);
			app_shell_putstr(" - ");
			app_shell_putstr(cmds[i]->short_help);
			i++;
		}
	}
	else
	{
		struct app_shell_command *cmd = app_shell_find_cmd(argv[1]);

		if(NULL != cmd)
		{
			app_shell_putstr(cmd->name);
			app_shell_putstr("\r\n");
			app_shell_putstr(cmd->long_help);
		}
		else
		{
			app_shell_putstr("can not find command : ");
			app_shell_putstr(argv[1]);
			app_shell_putstr("\r\n");
		}
	}

	return 0;
}

static int app_shell_clear_f(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	app_shell_clear();

	return 0;
}

static int app_shell_reboot_f(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	app_shell_putstr("[reboot] system reset\r\n");
	NVIC_SystemReset();

	return 0;
}

/* 按空白切分参数，line 会被就地写入 '\0' */
static int app_shell_split(char *line, char *argv[APP_SHELL_ARG_MAX])
{
	int argc = 0;
	char *p = line;

	while('\0' != *p)
	{
		if(argc >= APP_SHELL_ARG_MAX)
		{
			break;
		}

		argv[argc++] = p;

		while(('\0' != *p) && (' ' != *p) && ('\t' != *p))
		{
			p++;
		}
		if('\0' == *p)
		{
			break;
		}
		*p++ = '\0';

		while((' ' == *p) || ('\t' == *p))
		{
			p++;
		}
	}

	return argc;
}

int app_shell_exec(char *line)
{
	char *argv[APP_SHELL_ARG_MAX];
	struct app_shell_command *cmd;
	int argc;

	while((' ' == *line) || ('\t' == *line))
	{
		line++;
	}

	argc = app_shell_split(line, argv);
	if(0 == argc)
	{
		return 0;
	}

	cmd = app_shell_find_cmd(argv[0]);
	if(NULL == cmd)
	{
		app_shell_putstr(argv[0]);
		app_shell_putstr(" command not found.\r\n");
		return 0;
	}

	return cmd->function(argc, argv);
}
