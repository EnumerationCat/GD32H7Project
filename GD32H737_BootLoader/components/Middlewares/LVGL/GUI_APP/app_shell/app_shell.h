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
