#include "led.h"


/**
 * @brief  Initialize the LED GPIO pins
 * @param  None
 * @retval None
 */
void led_init(void)
{
    rcu_periph_clock_enable(LED0_GPIO_CLK);     /* 使能LED0时钟 */
    rcu_periph_clock_enable(LED1_GPIO_CLK);     /* 使能LED1时钟 */
    
    /* LED0引脚模式设置 */
    gpio_mode_set(LED0_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, LED0_GPIO_PIN);
    gpio_output_options_set(LED0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, LED0_GPIO_PIN);
  
    /* LED1引脚模式设置 */
    gpio_mode_set(LED1_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, LED1_GPIO_PIN);
    gpio_output_options_set(LED1_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, LED1_GPIO_PIN);
	  
	  LED0(1);                                    /* 关闭 LED0 */ 
	  LED1(1);                                    /* 关闭 LED1 */
}
