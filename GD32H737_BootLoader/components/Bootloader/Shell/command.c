#include "command.h"
#include "help_cmd.h"
#include "boot_cmd.h"
#include "string.h"

static struct command * g_commands[] = {
	&help_cmd,
	&boot_cmd,
	&jump_cmd,
	&exit_cmd,
	NULL
};

struct command ** get_cmds(void)
{
	return g_commands;
}

struct command *find_cmd(char *name)
{
	int i = 0;
	while (g_commands[i])
	{
		if (strcmp(g_commands[i]->name, name) == 0)
			return g_commands[i];
		i++;
	}
	return NULL;
}














