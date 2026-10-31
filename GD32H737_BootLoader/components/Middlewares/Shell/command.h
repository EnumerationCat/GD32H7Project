#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "types.h"

struct command {
  char *name;
  char *short_help;
  char *long_help;
  int (*function)(int argc, char **argv);
};

struct command ** get_cmds(void);
struct command *find_cmd(char *name);


#endif

