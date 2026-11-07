#ifndef __APP_SHELL_CMD_H__
#define __APP_SHELL_CMD_H__

struct app_shell_command {
    char *name;
    char *short_help;
    char *long_help;
    int (*function)(int argc, char **argv);
};

struct app_shell_command **app_shell_get_cmds(void);
struct app_shell_command *app_shell_find_cmd(const char *name);

/* 拆分参数、查表并执行一行命令；line 会被就地切分 */
int app_shell_exec(char *line);

#endif
