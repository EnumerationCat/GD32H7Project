/**
 ******************************************************************************
 * @file    sys.h
 * @author  GigaDevice Application Team
 * @brief   Header file for system driver
 *          This file contains system control functions, delay functions,
 *          and assembly function prototypes.
 * @version V1.0.0
 * @date    2025-xx-xx
 ******************************************************************************
 * @attention
 *
 * This software is provided "as is", without warranty of any kind.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_H
#define __SYS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "gd32h7xx.h"



/**
 * SYS_SUPPORT_OS用于定义系统文件夹是否支持OS
 * 0,不支持OS
 * 1,支持OS
 */
#define SYS_SUPPORT_OS  1

/* General Functions ---------------------------------------------------------*/
void sys_soft_reset(void);                  /* System software reset */
void sys_cache_enable(void);                /* Enable Cache */

/* Assembly Functions --------------------------------------------------------*/
void sys_wfi_set(void);                     /* Execute WFI instruction */
void sys_intx_disable(void);                /* Disable all interrupts */
void sys_intx_enable(void);                 /* Enable all interrupts */
void sys_msr_msp(uint32_t addr);            /* Set main stack pointer address */

/* Delay Functions ----------------------------------------------------------*/
void delay_init(uint16_t sysclk);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_H */