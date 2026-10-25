

/**
 ****************************************************************************************************
 * @file        timer.c
 * @version     V1.0
 * @brief       定时器中断 驱动代码
 ****************************************************************************************************

 * 实验平台:    GD32H737VMT6小系统板
 *
 ****************************************************************************************************
 */
 
#include "timer.h"
#include "led.h"
#include <stdint.h>
#include <stdio.h>


#include "lvgl.h"


/**
 * @brief       定时器TIMERX定时中断初始化函数
 * @note
 *              定时器的时钟来自APB1或APB2,如果CK_APBx = CK_AHB或CK_APBx = CK_AHB/2，
 *              则定时器的时钟等于CK_AHB（CK_TIMERx = CK_AHB）,而CK_AHB为300M, 所以定时器时钟 = 300Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void timerx_int_init(uint64_t arr, uint16_t psc)
{
    timer_parameter_struct timer_initpara;                      /* timer_initpara用于存放定时器的参数 */

    /* 使能外设定时器时钟 */ 
    rcu_periph_clock_enable(TIMERX_INT_CLK);                    /* 使能TIMERX的时钟 */

    /* 复位定时器 */
    timer_deinit(TIMERX_INT);                                   /* 复位TIMERX */
    timer_struct_para_init(&timer_initpara);                    /* 初始化timer_initpara为默认值 */

    /* 配置定时器参数 */
    timer_initpara.prescaler         = psc;                     /* 设置预分频值 */
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      /* 设置对齐模式为边沿对齐模式 */
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        /* 设置向上计数模式 */
    timer_initpara.period            = arr;                     /* 设置自动重装载值 */
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        /* 设置时钟分频因子 */
    timer_initpara.repetitioncounter = 0;                       /* 设置重复计数器值 */
    timer_init(TIMERX_INT, &timer_initpara);                    /* 根据参数初始化定时器 */

    /* 使能定时器及其中断 */
    timer_interrupt_flag_clear(TIMERX_INT, TIMER_INT_FLAG_UP);  /* 清除定时器更新中断标志 */
  
    timer_interrupt_enable(TIMERX_INT, TIMER_INT_UP);           /* 使能定时器的更新中断 */
    
    nvic_irq_enable(TIMERX_INT_IRQn, 1, 3);                     /* 配置NVIC设置优先级，抢占优先级1，响应优先级3 */
    
    timer_enable(TIMERX_INT);                                   /* 使能定时器TIMERX */
}

/**
 * @brief       定时器TIMERX中断服务函数
 * @param       无
 * @retval      无
 */

volatile unsigned long ulHighFrequencyTimerTicks;

void TIMERX_INT_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMERX_INT, TIMER_INT_FLAG_UP) == SET)     /* 判断定时器更新中断是否发生 */
    {
		//LED1_TOGGLE();

		ulHighFrequencyTimerTicks++;

    
        timer_interrupt_flag_clear(TIMERX_INT, TIMER_INT_FLAG_UP);          /* 清除定时器更新中断标志 */
    }
}

















