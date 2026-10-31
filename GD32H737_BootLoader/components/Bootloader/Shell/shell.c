#include "shell.h"
#include "bootloader.h"
#include "string.h"
#include <stdbool.h>


struct finsh_shell g_shell;
static char *finsh_prompt_custom = "msh >";

static void shell_putstr(const char *str)
{
    if (NULL != str)
    {
        boot_puts(str);
    }
}

static void shell_putchar(char ch)
{
    boot_putc((uint8_t)ch);
}

const char *finsh_get_prompt(void)
{
    return finsh_prompt_custom;
}


uint32_t finsh_get_prompt_mode(void)
{
    return g_shell.prompt_mode;
}


void finsh_set_prompt_mode(uint32_t prompt_mode)
{
    g_shell.prompt_mode = prompt_mode;
}

static int finsh_getchar(void)
{
    /* 非阻塞轮询：没收到数据返回 -1，主循环 continue 继续等 */
    return boot_getc();
}


void finsh_set_echo(uint32_t echo)
{
    g_shell.echo_mode = (uint8_t)echo;
}


uint32_t finsh_get_echo()
{
    return g_shell.echo_mode;
}




//void finsh_run_line(struct finsh_parser *parser, const char *line)
//{
//}

static bool shell_handle_history(struct finsh_shell *shell)
{
#if 1
    int i;
    shell_putstr("\r");

    for (i = 0; i <= 60; i++)
        shell_putchar(' ');
    shell_putstr("\r");

#else
    shell_putstr("\033[2K\r");
#endif
    shell_putstr(FINSH_PROMPT);
	shell_putstr(g_shell.line);
    return false;
}

static void shell_push_history(struct finsh_shell *shell)
{
    if (g_shell.line_position != 0)
    {
        /* push history */
        if (g_shell.history_count >= FINSH_HISTORY_LINES)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&g_shell.cmd_history[FINSH_HISTORY_LINES - 1], g_shell.line, FINSH_CMD_SIZE))
            {
                /* move history */
                int index;
                for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
                {
                    memcpy(&g_shell.cmd_history[index][0],
                           &g_shell.cmd_history[index + 1][0], FINSH_CMD_SIZE);
                }
                memset(&g_shell.cmd_history[index][0], 0, FINSH_CMD_SIZE);
                memcpy(&g_shell.cmd_history[index][0], g_shell.line, g_shell.line_position);

                /* it's the maximum history */
                g_shell.history_count = FINSH_HISTORY_LINES;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (g_shell.history_count == 0 || memcmp(&g_shell.cmd_history[g_shell.history_count - 1], g_shell.line, FINSH_CMD_SIZE))
            {
                g_shell.current_history = g_shell.history_count;
                memset(&g_shell.cmd_history[g_shell.history_count][0], 0, FINSH_CMD_SIZE);
                memcpy(&g_shell.cmd_history[g_shell.history_count][0], g_shell.line, g_shell.line_position);

                /* increase count and set current history position */
                g_shell.history_count ++;
            }
        }
    }
    g_shell.current_history = g_shell.history_count;
}

static void shell_help(void)
{
    shell_putstr("available commands:\r\n");
    shell_putstr("  help  - show this help\r\n");
    shell_putstr("  boot  - exit shell and jump to App\r\n");
    shell_putstr("  jump  - alias of boot\r\n");
    shell_putstr("  exit  - alias of boot\r\n");
}

/* 退出命令 boot / jump / exit 返回 1，交回 bootloader 继续跳 App */
static int shell_exec(const char *line)
{
    if (0 == strcmp(line, "help"))
    {
        shell_help();
        return 0;
    }

    if (0 == strcmp(line, "boot") ||
        0 == strcmp(line, "jump") ||
        0 == strcmp(line, "exit"))
    {
        return 1;
    }

    if ('\0' != line[0])
    {
        shell_putstr("unknown command\r\n");
    }

    return 0;
}

void shell(void)
{
    int ch;

    /* normal is echo mode */
    g_shell.echo_mode = 1;

    shell_putstr(FINSH_PROMPT);

    while (1)
    {
        ch = finsh_getchar();
        if (ch < 0)
        {
            continue;
        }

        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b)
        {
            g_shell.stat = WAIT_SPEC_KEY;
            continue;
        }
        else if (g_shell.stat == WAIT_SPEC_KEY)
        {
            if (ch == 0x5b)
            {
                g_shell.stat = WAIT_FUNC_KEY;
                continue;
            }

            g_shell.stat = WAIT_NORMAL;
        }
        else if (g_shell.stat == WAIT_FUNC_KEY)
        {
            g_shell.stat = WAIT_NORMAL;

            if (ch == 0x41) /* up key */
            {
                /* prev history */
                if (g_shell.current_history > 0)
                    g_shell.current_history --;
                else
                {
                    g_shell.current_history = 0;
                    continue;
                }

                /* copy the history command */
                memcpy(g_shell.line, &g_shell.cmd_history[g_shell.current_history][0],
                       FINSH_CMD_SIZE);
                g_shell.line_curpos = g_shell.line_position = strlen(g_shell.line);
                shell_handle_history(&g_shell);
                continue;
            }
            else if (ch == 0x42) /* down key */
            {
                /* next history */
                if (g_shell.current_history < g_shell.history_count - 1)
                    g_shell.current_history ++;
                else
                {
                    /* set to the end of history */
                    if (g_shell.history_count != 0)
                        g_shell.current_history = g_shell.history_count - 1;
                    else
                        continue;
                }

                memcpy(g_shell.line, &g_shell.cmd_history[g_shell.current_history][0],
                       FINSH_CMD_SIZE);
                g_shell.line_curpos = g_shell.line_position = strlen(g_shell.line);
                shell_handle_history(&g_shell);
                continue;
            }
            else if (ch == 0x44) /* left key */
            {
                if (g_shell.line_curpos)
                {
                    shell_putstr("\b");
                    g_shell.line_curpos --;
                }

                continue;
            }
            else if (ch == 0x43) /* right key */
            {
                if (g_shell.line_curpos < g_shell.line_position)
                {
                    shell_putchar(g_shell.line[g_shell.line_curpos]);
                    g_shell.line_curpos ++;
                }

                continue;
            }
        }

        /* received null or error */
        if (ch == '\0' || ch == 0xFF) continue;
        /* handle tab key */
        else if (ch == '\t')
        {
			/* 未实现 */
        }
        /* handle backspace key */
        else if (ch == 0x7f || ch == 0x08)
        {
            /* note that g_shell.line_curpos >= 0 */
            if (g_shell.line_curpos == 0)
                continue;

            g_shell.line_position--;
            g_shell.line_curpos--;

            if (g_shell.line_position > g_shell.line_curpos)
            {
                int i;

                memmove(&g_shell.line[g_shell.line_curpos],
                           &g_shell.line[g_shell.line_curpos + 1],
                           g_shell.line_position - g_shell.line_curpos);
                g_shell.line[g_shell.line_position] = 0;

                //shell_putstr("\b%s  \b", &g_shell.line[g_shell.line_curpos]);
				shell_putchar('\b');
				shell_putstr(&g_shell.line[g_shell.line_curpos]);
				shell_putstr("  \b");
				

                /* move the cursor to the origin position */
                for (i = g_shell.line_curpos; i <= g_shell.line_position; i++)
                    shell_putstr("\b");
            }
            else
            {
                shell_putstr("\b \b");
                g_shell.line[g_shell.line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n')
        {
            shell_push_history(&g_shell);
			if (g_shell.echo_mode)
				shell_putstr("\r\n");

            g_shell.line[g_shell.line_position] = '\0';
            if (shell_exec(g_shell.line))
            {
                return;
            }

            shell_putstr(FINSH_PROMPT);
            memset(g_shell.line, 0, sizeof(g_shell.line));
            g_shell.line_curpos = g_shell.line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (g_shell.line_position >= FINSH_CMD_SIZE)
            g_shell.line_position = 0;

        /* normal character */
        if (g_shell.line_curpos < g_shell.line_position)
        {
            int i;

            memmove(&g_shell.line[g_shell.line_curpos + 1],
                       &g_shell.line[g_shell.line_curpos],
                       g_shell.line_position - g_shell.line_curpos);
            g_shell.line[g_shell.line_curpos] = ch;
            if (g_shell.echo_mode)
                shell_putstr(&g_shell.line[g_shell.line_curpos]);

            /* move the cursor to new position */
            for (i = g_shell.line_curpos; i < g_shell.line_position; i++)
                shell_putstr("\b");
        }
        else
        {
            g_shell.line[g_shell.line_position] = ch;
            if (g_shell.echo_mode)
                shell_putchar(ch);
        }

        ch = 0;
        g_shell.line_position ++;
        g_shell.line_curpos++;
        if (g_shell.line_position >= FINSH_CMD_SIZE)
        {
            /* clear command line */
            g_shell.line_position = 0;
            g_shell.line_curpos = 0;
        }
    } /* end of device read */
}


