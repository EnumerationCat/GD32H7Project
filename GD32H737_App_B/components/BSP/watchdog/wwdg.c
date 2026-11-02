/**
 ****************************************************************************************************
 * @file        wwdgt.c
 * @version     V1.0
 * @brief       窗口看门狗 驱动代码
 ****************************************************************************************************
 */
 
#include "wwdg.h"
#include "led.h"


/**
 * @brief       初始化窗口看门狗
 * @param       tr   : T[6:0],计数器值
 * @param       wr   : W[6:0],窗口值
 * @note        fprer:分频系数,范围:WWDGT_CFG_PSC_DIV1~WWDGT_CFG_PSC_DIV8,表示1~8分频
 *              Fwwdg = PCLK3 / (4096 * 2^fprer). 一般PCLK3 = 150Mhz
 *              溢出时间 = (4096 * 2^fprer) * (tr - 0X3F) / PCLK3
 *              假设fprer = WWDGT_CFG_PSC_DIV8, tr = 7f, PCLK3 = 150Mhz
 *              则溢出时间 = 4096 * 8 * 64 / 150Mhz = 13.98ms
 * @retval      无
 */
void wwdgt_init(uint8_t tr, uint8_t wr, uint32_t fprer)
{
    rcu_periph_clock_enable(RCU_WWDGT);         /* 使能WWDGT时钟 */
  
    wwdgt_config(tr, wr, fprer);                /* 设置WWDGT计数器值、窗口值和预分频值 */
    
    wwdgt_enable();                             /* 使能窗口看门狗定时器 */
    
    wwdgt_flag_clear();                         /* 清除WWDGT提前唤醒中断标志位状态 */
    
    wwdgt_interrupt_enable();                   /* 使能窗口看门狗提前唤醒中断 */
  
    nvic_irq_enable(WWDGT_IRQn, 2, 3);          /* 抢占优先级2，响应优先级为3 */             
}
   
/**
 * @brief       窗口看门狗中断服务函数
 * @param       无
 * @retval      无
 */
void WWDGT_IRQHandler(void)
{ 
    WWDGT_CTL = 0x7F;                           /* 直接操作寄存器，及时设置WWDGT计数器值 */
    
    wwdgt_flag_clear();                         /* 清除WWDGT提前唤醒中断标志位状态 */
    
    LED1_TOGGLE();                              /* LED1闪烁 */
}

