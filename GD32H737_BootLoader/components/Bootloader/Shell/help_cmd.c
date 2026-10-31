/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      the first version for FinSH
 * 2006-05-08     Bernard      change finsh thread stack to 2048
 * 2006-06-03     Bernard      add support for skyeye
 * 2006-09-24     Bernard      remove the code related with hardware
 * 2010-01-18     Bernard      fix down then up key bug.
 * 2010-03-19     Bernard      fix backspace issue and fix device read in shell.
 * 2010-04-01     Bernard      add prompt output when start and remove the empty history
 * 2011-02-23     Bernard      fix variable section end issue of finsh shell
 *                             initialization when use GNU GCC compiler.
 * 2016-11-26     armink       add password authentication
 * 2018-07-02     aozima       add custom prompt support.
 */



#include "help_cmd.h"
#include "bootloader.h"

static int help_f(int argc, char **argv)
{
	int i = 0;
	struct command **cmds;
	struct command *cmd;

	cmds = get_cmds();
	
	if (argc == 1)
	{
		while (cmds[i])
		{
			boot_puts(cmds[i]->name);
			boot_puts("\t\t - \t");
			boot_puts(cmds[i]->short_help);
			i++;
		}
	}
	else if(argc == 2)
	{
		/* argv[0] = "help"
		 * argv[1] = "command_name"
		 */
		cmd = find_cmd(argv[1]);
		if (cmd)
		{
			boot_puts(cmd->name);
			boot_puts("\r\n");
			boot_puts(cmd->long_help);
		}
		else
		{
			boot_puts("can not find command : ");
			boot_puts(argv[1]);
			boot_puts("\r\n");
		}
	}
	
	return 0;
}

struct command help_cmd = {
	"help",
		
	"list all commands\r\n",
	
	"Usage: \r\n"
	"       help, list all commands\r\n"
	"       help command, show usage of 'command'\r\n",

	help_f,
};















