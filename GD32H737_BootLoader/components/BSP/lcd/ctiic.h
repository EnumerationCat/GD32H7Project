/**
 ****************************************************************************************************
 * @file        ctiic.h
 * @version     V1.0
 * @brief       电容触摸屏驱动-IIC通信部分 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H737VMT6小系统板
 *
 ****************************************************************************************************
 */	

#ifndef __CTIIC_H
#define __CTIIC_H

#include "sys.h"    


/******************************************************************************************/
/* CT_IIC 引脚 定义 */

#define CT_IIC_SCL_GPIO_PORT            GPIOB
#define CT_IIC_SCL_GPIO_PIN             GPIO_PIN_11
#define CT_IIC_SCL_GPIO_CLK             RCU_GPIOB   /* GPIOB时钟使能 */

#define CT_IIC_SDA_GPIO_PORT            GPIOB
#define CT_IIC_SDA_GPIO_PIN             GPIO_PIN_14
#define CT_IIC_SDA_GPIO_CLK             RCU_GPIOB   /* GPIOB时钟使能 */

/******************************************************************************************/

/* IO操作函数 */ 
#define CT_IIC_SCL(x)    do{ x ? \
                             gpio_bit_write(CT_IIC_SCL_GPIO_PORT, CT_IIC_SCL_GPIO_PIN, SET) : \
                             gpio_bit_write(CT_IIC_SCL_GPIO_PORT, CT_IIC_SCL_GPIO_PIN, RESET); \
                         }while(0)         /* SCL */ 

#define CT_IIC_SDA(x)    do{ x ? \
                             gpio_bit_write(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN, SET) : \
                             gpio_bit_write(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN, RESET); \
                         }while(0)         /* SDA */

#define CT_READ_SDA      gpio_input_bit_get(CT_IIC_SDA_GPIO_PORT, CT_IIC_SDA_GPIO_PIN)     /* 读取SDA */ 
 
/******************************************************************************************/

/* IIC所有操作函数 */
void ct_iic_init(void);                      /* 初始化IIC的IO口 */
void ct_iic_start(void);                     /* 发送IIC开始信号 */
void ct_iic_stop(void);                      /* 发送IIC停止信号 */
void ct_iic_ack(void);                       /* IIC发送ACK信号 */
void ct_iic_nack(void);                      /* IIC不发送ACK信号 */
uint8_t ct_iic_wait_ack(void);               /* IIC等待ACK信号 */
void ct_iic_send_byte(uint8_t txd);          /* IIC发送一个字节 */
uint8_t ct_iic_read_byte(uint8_t ack);       /* IIC读取一个字节 */

#endif







