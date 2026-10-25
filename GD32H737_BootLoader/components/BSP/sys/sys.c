#include "sys.h"




#if SYS_SUPPORT_OS
#include "FreeRTOS.h"
#include "task.h"  
#endif



/**
 * @brief       Execute WFI instruction (enter low-power mode after execution, wait for interrupt wake-up)
 * @param       None
 * @retval      None
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       Disable all interrupts (excluding fault and NMI interrupts)
 * @param       None
 * @retval      None
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       Enable all interrupts
 * @param       None
 * @retval      None
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       Set the main stack pointer address
 * @param       addr: Main stack top address
 * @retval      None
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);        /* Set stack top address */
}

/**
 * @brief       System software reset
 * @param       None
 * @retval      None
 */
void sys_soft_reset(void)
{
    NVIC_SystemReset();
}

/**
 * @brief       Enable system cache
 * @param       None
 * @retval      None
 */
void sys_cache_enable(void)
{
    SCB_EnableICache();     /* Enable I-Cache */
    SCB_EnableDCache();     /* Enable D-Cache */
    SCB->CACR |= 1 << 2;    /* Force D-Cache write-through; avoid problems in actual use if disabled */
}

static uint32_t g_fac_us = 0;      /* Microsecond delay multiplier */

/**
 * @brief     初始化延迟函数
 * @param     sysclk: 系统时钟频率, 即CPU频率(HCLK)，单位 Mhz
 * @retval    无
 */
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS                                      /* 如果需要支持OS. */
    uint32_t reload;
#endif
    SysTick->CTRL |= (1 << 2);                          /* SYSTICK使用系统时钟源,频率为HCLK */
    g_fac_us = sysclk ;                                 /* 不论是否使用OS,g_fac_us都需要使用,作为1us的基础时基 */

#if !SYS_SUPPORT_OS                                      
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;           
    SysTick->LOAD = 0XFFFFFF;                           
#endif



#if SYS_SUPPORT_OS                                      /* 如果需要支持OS. */
    reload = sysclk;                                    /* 每秒钟的计数次数 单位为M */
    reload *= 1000000 / configTICK_RATE_HZ;            /* 根据delay_ostickspersec设定溢出时间
                                                         * reload为24位寄存器,最大值:16777216
                                                         */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;          /* 开启SYSTICK中断 */
    SysTick->LOAD = reload;                             /* 每1/delay_ostickspersec秒中断一次 */
	  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;           /* 开启SYSTICK */
  
#endif
}



/**
 * @brief     延时nus
 * @note      无论是否使用OS, 都是用时钟摘取法来做us延时
 * @param     nus: 要延时的us数.
 * @note      nus取值范围: 0 ~ (2^32 / g_fac_us) (g_fac_us一般等于系统主频, 自行套入计算)
 * @retval    无
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload;
    reload = SysTick->LOAD;             /* LOAD的值 */
    ticks = nus * g_fac_us;             /* 需要的节拍数 */
  
    told = SysTick->VAL;                /* 刚进入时的计数器值 */
  
    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;    /* 这里注意一下SYSTICK是一个递减的计数器就可以了. */
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            
            told = tnow;
            
            if (tcnt >= ticks) 
            {
                break;                  /* 时间超过/等于要延迟的时间,则退出. */
            }
        }
    }   
}

/**
 * @brief       Delay for specified milliseconds
 * @param       nms: Number of milliseconds to delay (0 < nms <= (2^32 / g_fac_us / 1000))
 * @retval      None
 */
void delay_ms(uint16_t nms)
{  
		uint32_t i;

		for (i=0; i<nms; i++)
		{
		   delay_us(1000);
		}
}
