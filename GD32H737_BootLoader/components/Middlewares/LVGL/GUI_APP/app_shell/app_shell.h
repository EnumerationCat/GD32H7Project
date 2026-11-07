/**
 * @file app_shell.h
 *
 */

#ifndef APP_SHELL_H
#define APP_SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include "lvgl.h"

/**********************
 *      DEFINES
 **********************/

#define APP_SHELL_CMD_SIZE       64
#define APP_SHELL_HISTORY_LINES  10

/**********************
 *      TYPEDEFS
 **********************/

/* 输入状态机，布局与 bootloader 的 finsh_shell 一致 */
enum input_stat
{
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};

struct app_shell
{
    enum input_stat stat;

    uint8_t echo_mode:1;
    uint8_t prompt_mode:1;

    uint16_t current_history;
    uint16_t history_count;

    char cmd_history[APP_SHELL_HISTORY_LINES][APP_SHELL_CMD_SIZE + 1];

    char line[APP_SHELL_CMD_SIZE + 1];
    uint16_t line_position;
    uint16_t line_curpos;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void app_shell(void);

/* 串口接收任务把数据喂进来，页面定时刷新显示 */
void app_shell_rx_push(const uint8_t *data, uint32_t size);

/* 命令模块用的输出接口：同时写到串口和页面日志 */
void app_shell_putstr(const char *s);

/* 清空页面日志（clear 命令） */
void app_shell_clear(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_SHELL_H*/
