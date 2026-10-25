/**
 ******************************************************************************
 * @file    led.h
 * @author  GigaDevice Application Team
 * @brief   Header file for LED driver
 *          This file contains the pin definitions, control macros and
 *          function prototypes for LED configuration.
 * @version V1.0.0
 * @date    2025-xx-xx
 ******************************************************************************
 * @attention
 *
 * Hardware Connection:
 * LED0  ->  GPIOA Pin 2
 * LED1  ->  GPIOA Pin 3
 *
 * Logic Level:
 * 0 = Output LOW (RESET)
 * 1 = Output HIGH (SET)
 * Adjust the logic according to the actual hardware circuit (common anode/cathode)
 *
 * This software is provided "as is", without warranty of any kind.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "gd32h7xx.h"


#define LED0_GPIO_PORT                  GPIOA
#define LED0_GPIO_PIN                   GPIO_PIN_2
#define LED0_GPIO_CLK                   RCU_GPIOA     /* GPIOA时钟使能 */

#define LED1_GPIO_PORT                  GPIOA
#define LED1_GPIO_PIN                   GPIO_PIN_3
#define LED1_GPIO_CLK                   RCU_GPIOA     /* GPIOA时钟使能 */

/* -------------------------- LED Control Macros ---------------------------- */
/**
 * @brief  Turn LED0 ON or OFF
 * @param  x: 0 = RESET (LOW), 1 = SET (HIGH)
 * @retval None
 */
#define LED0(x)   do{ x ? \
                      gpio_bit_write(LED0_GPIO_PORT, LED0_GPIO_PIN, SET) : \
                      gpio_bit_write(LED0_GPIO_PORT, LED0_GPIO_PIN, RESET); \
                  }while(0)      


/**
 * @brief  Turn LED1 ON or OFF
 * @param  x: 0 = RESET (LOW), 1 = SET (HIGH)
 * @retval None
 */
#define LED1(x)   do{ x ? \
                      gpio_bit_write(LED1_GPIO_PORT, LED1_GPIO_PIN, SET) : \
                      gpio_bit_write(LED1_GPIO_PORT, LED1_GPIO_PIN, RESET); \
                  }while(0)     



/**
 * @brief  Toggle LED0 state (ON <-> OFF)
 * @param  None
 * @retval None
 */
#define LED0_TOGGLE()   do{ gpio_bit_toggle(LED0_GPIO_PORT, LED0_GPIO_PIN); }while(0)      /* 翻转LED0 */

/**
 * @brief  Toggle LED1 state (ON <-> OFF)
 * @param  None
 * @retval None
 */
#define LED1_TOGGLE()   do{ gpio_bit_toggle(LED1_GPIO_PORT, LED1_GPIO_PIN); }while(0)      /* 翻转LED1 */

/* -------------------------- Function Prototypes --------------------------- */

void led_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __LED_H */