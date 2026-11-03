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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_SHELL_H*/
