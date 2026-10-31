#include "help_cmd.h"
#include "shell.h"

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
			shell_putstr(cmds[i]->name);
			shell_putstr("\t\t - \t");
			shell_putstr(cmds[i]->short_help);
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
			shell_putstr(cmd->name);
			shell_putstr("\r\n");
			shell_putstr(cmd->long_help);
		}
		else
		{
			shell_putstr("can not find command : ");
			shell_putstr(argv[1]);
			shell_putstr("\r\n");
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















