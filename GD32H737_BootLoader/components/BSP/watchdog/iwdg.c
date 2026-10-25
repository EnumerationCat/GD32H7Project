/**
 ****************************************************************************************************
 * @file        fwdgt.c
 * @version     V1.0
 * @brief       独立看门狗 驱动代码
 ****************************************************************************************************/
 
#include "iwdg.h"


/**
 * @brief       初始化独立看门狗 
 * @param       prer  :分频数:FWDGT_PSC_DIV4~FWDGT_PSC_DIV256,对应4~256分频
 *   @arg       分频因子 = 4 * 2^prer. 但最大值只能是256!
 * @param       rlr   :自动重装载值,0~0XFFF. 
 * @note        时间计算(大概):Tout=((4 * 2^prer) * rlr) / 32 (ms). 
 * @retval      无
 */
void fwdgt_init(uint8_t prer, uint16_t rlr)
{
    /* 配置独立看门狗定时器 */
    fwdgt_write_enable();          /* 使能对寄存器FWDGT_PSC和FWDGT_RLD的写操作 */
    
    fwdgt_config(rlr, prer);       /* 设置FWDGT重装载值和预分频值 */
      
    fwdgt_counter_reload();        /* 将FWDGT_RLD寄存器的值重装载FWDGT计数器 */
    
    fwdgt_enable();                /* 使能独立看门狗定时器 */

}
   
/**
 * @brief       喂独立看门狗
 * @param       无
 * @retval      无
 */
void fwdgt_feed(void)
{   
    fwdgt_counter_reload();        /* 喂狗 */
}
