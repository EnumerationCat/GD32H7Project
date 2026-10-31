#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdint.h>
#include <stdbool.h>

#ifndef FINSH_CMD_SIZE
#define FINSH_CMD_SIZE      80
#endif

#define FINSH_OPTION_ECHO   0x01


#define FINSH_PROMPT        finsh_get_prompt()
const char* finsh_get_prompt(void);
int finsh_set_prompt(const char * prompt);

#define FINSH_HISTORY_LINES 5


enum input_stat
{
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};
struct finsh_shell
{

    enum input_stat stat;

    uint8_t echo_mode:1;
    uint8_t prompt_mode: 1;

    uint16_t current_history;
    uint16_t history_count;

    char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];


    char line[FINSH_CMD_SIZE + 1];
    uint16_t line_position;
    uint16_t line_curpos;
};

/* 命令行主循环；执行 boot / jump / exit 命令后返回（不再跳 App 之外的用途） */
void shell(void);

/* shell 输出，供命令实现（如 help）使用 */
void shell_putstr(const char *str);

void finsh_set_echo(uint32_t echo);
uint32_t finsh_get_echo(void);

int finsh_system_init(void);
void finsh_set_device(const char* device_name);
const char* finsh_get_device(void);

uint32_t finsh_get_prompt_mode(void);
void finsh_set_prompt_mode(uint32_t prompt_mode);


#endif

