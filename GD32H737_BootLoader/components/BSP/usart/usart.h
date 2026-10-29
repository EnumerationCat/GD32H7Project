/**
 ****************************************************************************************************					 
 * @file        usart.h
 * @version     V1.1
 * @brief       串口初始化代码，支持printf            
 ****************************************************************************************************
 *
 * V1.1
 * 修改SYS_SUPPORT_OS部分代码, 包含头文件改成:"os.h"
 *
 ****************************************************************************************************
 */	

#ifndef __USART_H
#define __USART_H


#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/

#include "stdio.h"
#include "sys.h"

/*******************************************************************************************************/
/* 引脚和串口 定义 */

#define USART_TX_GPIO_PORT              GPIOA
#define USART_TX_GPIO_PIN               GPIO_PIN_9
#define USART_TX_GPIO_AF                GPIO_AF_7
#define USART_TX_GPIO_CLK               RCU_GPIOA   /* GPIOA时钟使能 */

#define USART_RX_GPIO_PORT              GPIOA
#define USART_RX_GPIO_PIN               GPIO_PIN_10
#define USART_RX_GPIO_AF                GPIO_AF_7
#define USART_RX_GPIO_CLK               RCU_GPIOA   /* GPIOA时钟使能 */

#define USART_UX                        USART0
#define USART_UX_IRQn                   USART0_IRQn
#define USART_UX_IRQHandler             USART0_IRQHandler
#define USART_UX_CLK                    RCU_USART0  /* USART0 时钟使能 */

/* 串口 DMA 配置：GD32H7 的请求由 DMAMUX 路由，通道可任选 */
#define USART_DMA                       DMA0
#define USART_DMA_RX_CH                 DMA_CH1
#define USART_DMA_TX_CH                 DMA_CH2
#define USART_DMA_RX_IRQn               DMA0_Channel1_IRQn
#define USART_DMA_TX_IRQn               DMA0_Channel2_IRQn
#define USART_DMA_RX_IRQHandler         DMA0_Channel1_IRQHandler
#define USART_DMA_TX_IRQHandler         DMA0_Channel2_IRQHandler
#define USART_DMA_REQUEST_RX            DMA_REQUEST_USART0_RX
#define USART_DMA_REQUEST_TX            DMA_REQUEST_USART0_TX

#define USART_EN_RX                 1               /* 使能（1）/禁止（0）串口接收 */
/* 串口printf重定向配置：1=使能开启，0=禁用关闭 */
#define USART_SUPPORT_PRINTF    1

#if USART_SUPPORT_PRINTF
#define LOG_INFO(fmt, ...)  printf("%s:%s:%d: " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


#define UART_DMA_MODE        1      /* DMA + IDLE 空闲中断接收，DMA 发送 */
#define UART_INTERRUPT_MODE  0      /* 接收缓冲区非空中断接收，发送空中断发送 */
#define UART_DELAY_MODE      0      /* 查询方式收发 */

#if ( (UART_INTERRUPT_MODE + UART_DMA_MODE + UART_DELAY_MODE) != 1 )
    #error "just one mode can be selected"
#endif



#define UART_BUFFER_SIZE     512
#define UART_RX_QUEUE_LEN    512
/*******************************************************************************************************/

void usart_init(uint32_t baudrate);                 /* 串口初始化函数 */

/* GD32 版 UART 句柄：SPL 没有 HAL 句柄，发送状态只能自己存。
   字段对应 STM32 HAL 的 UART_HandleTypeDef（pTxBuffPtr / TxXferSize / TxXferCount） */
typedef struct
{
    uint32_t          Instance;     /* USART 外设基址 */
    uint8_t          *pTxBuffPtr;   /* 发送缓冲区指针 */
    volatile uint16_t TxXferSize;   /* 本次发送长度 */
    volatile uint16_t TxXferCount;  /* 剩余待发送字节数 */
} UART_HandleTypeDef;

struct UART_Device {
    char *name;
    int (*Init)(struct UART_Device *pDev, int baud, int datas, char parity, int stop);
    int (*DeInit)(struct UART_Device *pDev);
    int (*Send)(struct UART_Device *pDev, uint8_t *datas, int len, int timeout_ms);
    int (*Recv)(struct UART_Device *pDev, uint8_t *data, int timeout_ms);
    void *priv_data;
};

typedef struct UART_Device *PUART_Device;

struct UART_Device *GetUARTDevice(char *name);



#ifdef __cplusplus
}


#endif

#endif
