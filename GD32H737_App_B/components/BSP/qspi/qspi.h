/**
 ****************************************************************************************************
 * @file        qspi.h
 * @version     V1.0
 * @brief       QSPI 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H737VMT6小系统板
 *
 ****************************************************************************************************
 */		

#ifndef __QSPI_H
#define __QSPI_H

#include "sys.h"


/******************************************************************************************/
/* QSPI 相关 引脚 定义 */

#define OSPIM_P0_SCK_GPIO_PORT          GPIOB
#define OSPIM_P0_SCK_GPIO_PIN           GPIO_PIN_2
#define OSPIM_P0_SCK_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_SCK_GPIO_CLK           RCU_GPIOB   /* GPIOB时钟使能 */

#define OSPIM_P0_CSN_GPIO_PORT          GPIOB
#define OSPIM_P0_CSN_GPIO_PIN           GPIO_PIN_10
#define OSPIM_P0_CSN_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_CSN_GPIO_CLK           RCU_GPIOB   /* GPIOB时钟使能 */

#define OSPIM_P0_IO0_GPIO_PORT          GPIOD
#define OSPIM_P0_IO0_GPIO_PIN           GPIO_PIN_11
#define OSPIM_P0_IO0_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_IO0_GPIO_CLK           RCU_GPIOD   /* GPIOD时钟使能 */

#define OSPIM_P0_IO1_GPIO_PORT          GPIOD
#define OSPIM_P0_IO1_GPIO_PIN           GPIO_PIN_12
#define OSPIM_P0_IO1_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_IO1_GPIO_CLK           RCU_GPIOD   /* GPIOD时钟使能 */

#define OSPIM_P0_IO2_GPIO_PORT          GPIOE
#define OSPIM_P0_IO2_GPIO_PIN           GPIO_PIN_2
#define OSPIM_P0_IO2_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_IO2_GPIO_CLK           RCU_GPIOE   /* GPIOE时钟使能 */

#define OSPIM_P0_IO3_GPIO_PORT          GPIOD
#define OSPIM_P0_IO3_GPIO_PIN           GPIO_PIN_13
#define OSPIM_P0_IO3_GPIO_AF            GPIO_AF_9
#define OSPIM_P0_IO3_GPIO_CLK           RCU_GPIOD   /* GPIOD时钟使能 */

#define OSPI_INTERFACE             OSPI0            /* 使用的OSPI接口 */

extern ospi_parameter_struct g_ospi_struct;         /* OSPI参数结构体 */

/******************************************************************************************/

void ospi_flash_init(uint32_t ospi_periph, ospi_parameter_struct *ospi_struct);                         /* 初始化OSPI接口 */   
void ospi_send_command(uint32_t ins, uint32_t addr, uint32_t datalen, uint16_t mode, uint8_t dmcycle);  /* OSPI发送命令 */

#endif
























