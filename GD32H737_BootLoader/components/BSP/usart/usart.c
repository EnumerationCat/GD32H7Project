/**
 ****************************************************************************************************					 
 * @file        usart.c
 * @version     V1.1
 * @brief       串口初始化代码，支持printf            
 ****************************************************************************************************
 */
 
#include "sys.h"
#include "usart.h"


#if USART_SUPPORT_PRINTF

/******************************************************************************************/
/* GCC 编译器 printf 重定向到串口，无需 MicroLIB */
#include <stdio.h>
#include <sys/stat.h>

/* 关闭半主机模式（GCC 必须） */
void _exit(int x)
{
    while(1);
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _read(int file, char *ptr, int len)
{
    return 0;
}

/*GCC 重定向 write 函数，printf 最终调用此函数 */
int _write(int file, char *ptr, int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        /* 等待串口发送完成 */
        while (RESET == usart_flag_get(USART_UX, USART_FLAG_TC));
        /* 发送单个字符 */
        usart_data_transmit(USART_UX, (uint8_t)ptr[i]);
    }
    return len;
}
/******************************************************************************************/
#endif


#if USART_EN_RX       /* 如果使能了接收 */

/* 接收缓冲, 最大USART_REC_LEN个字节. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  接收状态
 *  bit15，      接收完成标志
 *  bit14，      接收到0x0d
 *  bit13~0，    接收到的有效字节数目
 */
uint16_t g_usart_rx_sta = 0;

/**
 * @brief       串口X中断服务函数
 * @param       无
 * @retval      无
 */
void USART_UX_IRQHandler(void)
{
    uint8_t rxdata;

   
	if (usart_interrupt_flag_get(USART_UX, USART_INT_FLAG_RBNE) != RESET)   /* 接收到数据 */
	{
        rxdata = usart_data_receive(USART_UX);	         /* 读取接收到的数据 */
        
        if ((g_usart_rx_sta & 0x8000) == 0)              /* 接收未完成? */
        {
            if (g_usart_rx_sta & 0x4000)                 /* 接收到了0x0d?  */
            { 
                if (rxdata != 0x0a)                      /* 接收到了0x0a? (必须先接收到到0x0d,才检查0x0a) */
                { 
                    g_usart_rx_sta = 0;                  /* 接收错误,重新开始 */
                }
                else 
                {
                    g_usart_rx_sta |= 0x8000;	           /* 收到了0x0a,标记接收完成了 */ 
                }
            }
            else                                         /* 还没收到0X0d  */
            {	
                if (rxdata == 0x0d)
                {
                    g_usart_rx_sta |= 0x4000;            /* 标记接收到了0x0d  */                            
                } 
                else
                {
                    g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = rxdata;              /* 存储数据到 g_usart_rx_buf */
                    g_usart_rx_sta++;
                  
                    if (g_usart_rx_sta > (USART_REC_LEN - 1))g_usart_rx_sta = 0;   /* 接收数据溢出,重新开始接收 */ 
                }		 
            }
        }   		 
     } 

    usart_flag_clear(USART_UX, USART_FLAG_ORERR);        /* 清除溢出错误标志, 否则可能会卡死在串口中断服务函数里面 */   

}
#endif

/**
 * @brief       串口X初始化函数
 * @param       bound: 波特率, 根据自己需要设置波特率值
 * @retval      无
 */
void usart_init(uint32_t baudrate)
{
    /* IO 及 时钟配置 */
    rcu_periph_clock_enable(USART_TX_GPIO_CLK);     /* 使能串口TX脚时钟 */
    rcu_periph_clock_enable(USART_RX_GPIO_CLK);     /* 使能串口RX脚时钟 */
    rcu_periph_clock_enable(USART_UX_CLK);          /* 使能串口时钟 */

    /* 设置USARTx_TX引脚的复用功能选择 */
    gpio_af_set(USART_TX_GPIO_PORT, USART_TX_GPIO_AF, USART_TX_GPIO_PIN);

    /* 设置USARTx_RX引脚的复用功能选择 */
    gpio_af_set(USART_RX_GPIO_PORT, USART_RX_GPIO_AF, USART_RX_GPIO_PIN);

    /* USARTx_TX引脚的模式设置 */
    gpio_mode_set(USART_TX_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_TX_GPIO_PIN);
    gpio_output_options_set(USART_TX_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, USART_TX_GPIO_PIN);

    /* USARTx_RX引脚的模式设置 */
    gpio_mode_set(USART_RX_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_RX_GPIO_PIN);
    gpio_output_options_set(USART_RX_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, USART_RX_GPIO_PIN);

    /* 配置USART的参数 */
    usart_deinit(USART_UX);                                 /* 复位USARTx */
    usart_baudrate_set(USART_UX, baudrate);                 /* 设置波特率 */
    usart_stop_bit_set(USART_UX, USART_STB_1BIT);           /* 一个停止位 */
    usart_word_length_set(USART_UX, USART_WL_8BIT);         /* 字长为8位数据格式 */
    usart_parity_config(USART_UX, USART_PM_NONE);           /* 无奇偶校验位 */
    usart_transmit_config(USART_UX, USART_TRANSMIT_ENABLE); /* 使能发送 */
    
#if USART_EN_RX                                             /* 如果使能了接收 */
    usart_receive_config(USART_UX, USART_RECEIVE_ENABLE);   /* 使能接收 */
    usart_interrupt_enable(USART_UX, USART_INT_RBNE);       /* 使能接收缓冲区非空中断 */    
    /* 配置NVIC，并设置中断优先级 */
    nvic_irq_enable(USART_UX_IRQn, 3, 3);                   /* 组2，抢占优先级3，子优先级3 */
#endif

    usart_enable(USART_UX);	                                /* 使能串口 */
}
