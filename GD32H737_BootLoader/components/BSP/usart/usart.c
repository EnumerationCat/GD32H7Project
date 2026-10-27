/**
 ****************************************************************************************************
 * @file        usart.c
 * @version     V1.2
 * @brief       串口初始化代码，支持printf；并参考 uart_device 实现 GD32 的 UART_Device 设备抽象
 ****************************************************************************************************
 */

#include "sys.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>
#include <stdint.h>

/* 设备是否已完成初始化（互斥锁/信号量/队列是否已建好） */
static uint8_t g_uart_dev_ready = 0;


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

    /* 调度器已运行、设备已初始化，且不在中断/临界区内：走 UART_Device 发送，
       与任务里的 Send 共用同一把发送锁，避免 DMA 发送和轮询写 TDATA 互相踩 */
    if ((0U == __get_IPSR()) && (0U == __get_PRIMASK()) && (0U == __get_BASEPRI()) &&
        (0U != g_uart_dev_ready) && (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()))
    {
        struct UART_Device *dev = GetUARTDevice("gd32_uart0");

        if (NULL != dev)
        {
            dev->Send(dev, (uint8_t *)ptr, len, 100);
            return len;
        }
    }

    /* 启动阶段/中断/临界区内的兜底：直接轮询发送数据寄存器空。
       不能等 TC：UART_Device 的收发会清零 TC，清掉后 TC 不会再自动置位，
       这里等 TC 就会永久卡死 */
    for(i = 0; i < len; i++)
    {
        while (RESET == usart_flag_get(USART_UX, USART_FLAG_TBE));
        usart_data_transmit(USART_UX, (uint8_t)ptr[i]);
    }
    return len;
}
/******************************************************************************************/
#endif


/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
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
    /* 接收中断/NVIC 由 gd32_uart_init 按设备模式统一配置 */
#endif

    usart_enable(USART_UX);	                                /* 使能串口 */
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/*                        UART_Device 设备抽象（参考 STM32 的 uart_device.c）                     */
//////////////////////////////////////////////////////////////////////////////////////////////////

#define UART_DMA_MODE        1      /* DMA + IDLE 空闲中断接收，DMA 发送 */
#define UART_INTERRUPT_MODE  0      /* 接收缓冲区非空中断接收，发送空中断发送 */
#define UART_DELAY_MODE      0      /* 查询方式收发 */

#if ( (UART_INTERRUPT_MODE + UART_DMA_MODE + UART_DELAY_MODE) != 1 )
    #error "just one mode can be selected"
#endif

#define UART_BUFFER_SIZE     512
#define UART_RX_QUEUE_LEN    512

struct UART_Data {
    SemaphoreHandle_t xTxSem;   /* 发送完成信号量 */
    SemaphoreHandle_t xTxMutex; /* 发送互斥锁 */
    QueueHandle_t     xRxQueue; /* 接收队列，每项 1 字节 */
    /* 32 字节对齐：AXI SRAM 是 cacheable，DMA 前后需要按 cache 行做维护 */
    uint8_t           rxdata[UART_BUFFER_SIZE] __attribute__((aligned(32)));
    volatile int      read_index;
#if UART_INTERRUPT_MODE
    uint8_t          *txbuf;    /* 中断发送缓冲，调用者需等待发送完成 */
    int               txlen;
    volatile int      txidx;
#endif
};

static struct UART_Data   g_gd32_uart0_data;
static struct UART_Device gd32_uart0;

static int gd32_uart_init(struct UART_Device *pDev, int baud, int datas, char parity, int stop);
static int gd32_uart_send(struct UART_Device *pDev, uint8_t *datas, int len, int timeout_ms);
static int gd32_uart_recv(struct UART_Device *pDev, uint8_t *data, int timeout_ms);


#if UART_DMA_MODE

/* 配置 USART0 的 RX/TX DMA 通道 */
static void gd32_uart_dma_config(void)
{
    dma_single_data_parameter_struct dma_cfg;

    rcu_periph_clock_enable(RCU_DMA0);
    /* GD32H7 的 DMAMUX 有独立 AHB1 时钟，不开的话请求映射不生效，DMA 收不到外设请求 */
    rcu_periph_clock_enable(RCU_DMAMUX);

    /* ---------- 接收通道 ---------- */
    dma_deinit(USART_DMA, USART_DMA_RX_CH);
    dma_single_data_para_struct_init(&dma_cfg);
    dma_cfg.request             = USART_DMA_REQUEST_RX;
    dma_cfg.periph_addr         = (uint32_t)&USART_RDATA(USART_UX);
    dma_cfg.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_cfg.memory0_addr        = (uint32_t)g_gd32_uart0_data.rxdata;
    dma_cfg.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_cfg.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_cfg.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_cfg.direction           = DMA_PERIPH_TO_MEMORY;
    dma_cfg.number              = UART_BUFFER_SIZE;
    dma_cfg.priority            = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(USART_DMA, USART_DMA_RX_CH, &dma_cfg);
    dma_interrupt_enable(USART_DMA, USART_DMA_RX_CH, DMA_INT_FTF);
    nvic_irq_enable(USART_DMA_RX_IRQn, 6, 0);

    /* ---------- 发送通道 ---------- */
    dma_deinit(USART_DMA, USART_DMA_TX_CH);
    dma_single_data_para_struct_init(&dma_cfg);
    dma_cfg.request             = USART_DMA_REQUEST_TX;
    dma_cfg.periph_addr         = (uint32_t)&USART_TDATA(USART_UX);
    dma_cfg.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_cfg.memory0_addr        = 0;
    dma_cfg.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_cfg.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_cfg.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_cfg.direction           = DMA_MEMORY_TO_PERIPH;
    dma_cfg.number              = 0;
    dma_cfg.priority            = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(USART_DMA, USART_DMA_TX_CH, &dma_cfg);
    dma_interrupt_enable(USART_DMA, USART_DMA_TX_CH, DMA_INT_FTF);
    nvic_irq_enable(USART_DMA_TX_IRQn, 6, 0);
}

/* 重新装载接收 DMA（每次空闲/接收满后调用） */
static void gd32_uart_rx_dma_restart(void)
{
    dma_channel_disable(USART_DMA, USART_DMA_RX_CH);
    dma_flag_clear(USART_DMA, USART_DMA_RX_CH, DMA_FLAG_FTF | DMA_FLAG_HTF);
    dma_memory_address_config(USART_DMA, USART_DMA_RX_CH, DMA_MEMORY_0,
                              (uint32_t)g_gd32_uart0_data.rxdata);
    dma_transfer_number_config(USART_DMA, USART_DMA_RX_CH, UART_BUFFER_SIZE);
    dma_channel_enable(USART_DMA, USART_DMA_RX_CH);
    g_gd32_uart0_data.read_index = 0;
}

/* 把 DMA 收到的数据写入队列，然后重新启动接收 */
static void gd32_uart_rx_dma_process(BaseType_t *woken)
{
    struct UART_Data *uart_data = &g_gd32_uart0_data;
    int rx_len;
    int i;

    dma_channel_disable(USART_DMA, USART_DMA_RX_CH);
    /* DMA 直接写内存，CPU 读之前必须让 D-Cache 失效，否则可能读到旧数据 */
    SCB_InvalidateDCache_by_Addr((uint32_t *)uart_data->rxdata, UART_BUFFER_SIZE);

    rx_len = UART_BUFFER_SIZE - (int)dma_transfer_number_get(USART_DMA, USART_DMA_RX_CH);
    if (rx_len < 0)                 rx_len = 0;
    if (rx_len > UART_BUFFER_SIZE)  rx_len = UART_BUFFER_SIZE;

    for (i = uart_data->read_index; i < rx_len; i++)
    {
        xQueueSendFromISR(uart_data->xRxQueue, &uart_data->rxdata[i], woken);
    }

    gd32_uart_rx_dma_restart();
}

#endif /* UART_DMA_MODE */


/* 串口全局中断：设备模式下处理 IDLE / TC（中断模式下处理 RBNE / TBE） */
void USART_UX_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;

    if (g_uart_dev_ready)
    {
#if !UART_DELAY_MODE
        struct UART_Data *uart_data = &g_gd32_uart0_data;
#endif

#if UART_DMA_MODE
        if (usart_interrupt_flag_get(USART_UX, USART_INT_FLAG_IDLE) != RESET)
        {
            usart_interrupt_flag_clear(USART_UX, USART_INT_FLAG_IDLE);
            gd32_uart_rx_dma_process(&woken);
        }
#endif

#if UART_INTERRUPT_MODE
        if (usart_interrupt_flag_get(USART_UX, USART_INT_FLAG_RBNE) != RESET)
        {
            uint8_t rxdata = (uint8_t)usart_data_receive(USART_UX);
            xQueueSendFromISR(uart_data->xRxQueue, &rxdata, &woken);
        }

        if (usart_interrupt_flag_get(USART_UX, USART_INT_FLAG_TBE) != RESET)
        {
            if (uart_data->txidx < uart_data->txlen)
            {
                usart_data_transmit(USART_UX, uart_data->txbuf[uart_data->txidx++]);
            }
            if (uart_data->txidx >= uart_data->txlen)
            {
                usart_interrupt_disable(USART_UX, USART_INT_TBE);
                usart_interrupt_enable(USART_UX, USART_INT_TC);
            }
        }
#endif

#if !UART_DELAY_MODE
        /* DMA 发送完成(FTF)后才使能 TC 中断，这里表示真正发送完毕 */
        if (usart_interrupt_flag_get(USART_UX, USART_INT_FLAG_TC) != RESET)
        {
            usart_interrupt_flag_clear(USART_UX, USART_INT_FLAG_TC);
            usart_interrupt_disable(USART_UX, USART_INT_TC);
            xSemaphoreGiveFromISR(uart_data->xTxSem, &woken);
        }
#endif
        usart_flag_clear(USART_UX, USART_FLAG_ORERR);
    }

    portYIELD_FROM_ISR(woken);
}


#if UART_DMA_MODE

/* 接收 DMA 传输完成（缓冲满） */
void USART_DMA_RX_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;

    if (dma_interrupt_flag_get(USART_DMA, USART_DMA_RX_CH, DMA_INT_FLAG_FTF) != RESET)
    {
        dma_interrupt_flag_clear(USART_DMA, USART_DMA_RX_CH, DMA_INT_FLAG_FTF);
        gd32_uart_rx_dma_process(&woken);
    }

    portYIELD_FROM_ISR(woken);
}

/* 发送 DMA 传输完成：数据已全部写入 TDATA，再等 USART 的 TC 表示线上发完 */
void USART_DMA_TX_IRQHandler(void)
{
    if (dma_interrupt_flag_get(USART_DMA, USART_DMA_TX_CH, DMA_INT_FLAG_FTF) != RESET)
    {
        dma_interrupt_flag_clear(USART_DMA, USART_DMA_TX_CH, DMA_INT_FLAG_FTF);
        dma_channel_disable(USART_DMA, USART_DMA_TX_CH);
        usart_dma_transmit_config(USART_UX, USART_TRANSMIT_DMA_DISABLE);
        usart_interrupt_enable(USART_UX, USART_INT_TC);
    }
}

#endif /* UART_DMA_MODE */


static int gd32_uart_init(struct UART_Device *pDev, int baud, int datas, char parity, int stop)
{
    struct UART_Data *uart_data = pDev->priv_data;

    /* main 里已用设备方式初始化过，重复调用直接返回，避免重复建对象和重启 DMA */
    if (g_uart_dev_ready)
    {
        return 0;
    }

    (void)datas;
    (void)parity;
    (void)stop;

    usart_init((uint32_t)baud);

    uart_data->xTxMutex = xSemaphoreCreateMutex();
#if !UART_DELAY_MODE
    uart_data->xTxSem   = xSemaphoreCreateBinary();
    uart_data->xRxQueue = xQueueCreate(UART_RX_QUEUE_LEN, 1);
    if ((NULL == uart_data->xTxMutex) || (NULL == uart_data->xTxSem) || (NULL == uart_data->xRxQueue))
    {
        return -1;
    }
#else
    if (NULL == uart_data->xTxMutex)
    {
        return -1;
    }
#endif

#if UART_DMA_MODE
    /* DMA 接收时不能开 RBNE 中断，否则会与 DMA 抢数据 */
    usart_interrupt_disable(USART_UX, USART_INT_RBNE);
    gd32_uart_dma_config();
    usart_flag_clear(USART_UX, USART_FLAG_TC);

    /* 优先级数值必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY(5)，才能调用 FromISR API */
    nvic_irq_enable(USART_UX_IRQn, 6, 0);

    g_uart_dev_ready = 1;
    usart_interrupt_enable(USART_UX, USART_INT_IDLE);
    usart_dma_receive_config(USART_UX, USART_RECEIVE_DMA_ENABLE);
    gd32_uart_rx_dma_restart();
#endif

#if UART_INTERRUPT_MODE
    nvic_irq_enable(USART_UX_IRQn, 6, 0);
    g_uart_dev_ready = 1;
    usart_interrupt_enable(USART_UX, USART_INT_RBNE);
#endif

#if UART_DELAY_MODE
    usart_interrupt_disable(USART_UX, USART_INT_RBNE);
    nvic_irq_disable(USART_UX_IRQn);
    g_uart_dev_ready = 1;
#endif

    return 0;
}

static int gd32_uart_send(struct UART_Device *pDev, uint8_t *datas, int len, int timeout_ms)
{
    struct UART_Data *uart_data = pDev->priv_data;

    /* 加互斥锁，避免多任务同时发送 */
    if (pdTRUE != xSemaphoreTake(uart_data->xTxMutex, (TickType_t)timeout_ms))
    {
        return -1;
    }

#if UART_DELAY_MODE
    for (int i = 0; i < len; i++)
    {
        while (RESET == usart_flag_get(USART_UX, USART_FLAG_TBE));
        usart_data_transmit(USART_UX, datas[i]);
    }
    while (RESET == usart_flag_get(USART_UX, USART_FLAG_TC));
    xSemaphoreGive(uart_data->xTxMutex);
    return 0;
#else
    /* 清掉可能残留的发送完成信号量 */
    xSemaphoreTake(uart_data->xTxSem, 0);

#if UART_DMA_MODE
    usart_flag_clear(USART_UX, USART_FLAG_TC);
    dma_channel_disable(USART_DMA, USART_DMA_TX_CH);
    dma_flag_clear(USART_DMA, USART_DMA_TX_CH, DMA_FLAG_FTF | DMA_FLAG_HTF);
    dma_memory_address_config(USART_DMA, USART_DMA_TX_CH, DMA_MEMORY_0, (uint32_t)datas);
    dma_transfer_number_config(USART_DMA, USART_DMA_TX_CH, (uint32_t)len);
    /* 先把 CPU 写在 cache 里的数据回写到内存，否则 DMA 可能发出旧数据 */
    SCB_CleanDCache_by_Addr((uint32_t *)datas, len);
    usart_dma_transmit_config(USART_UX, USART_TRANSMIT_DMA_ENABLE);
    dma_channel_enable(USART_DMA, USART_DMA_TX_CH);
#endif

#if UART_INTERRUPT_MODE
    usart_flag_clear(USART_UX, USART_FLAG_TC);
    uart_data->txbuf = datas;
    uart_data->txlen = len;
    uart_data->txidx = 0;
    usart_interrupt_enable(USART_UX, USART_INT_TBE);
#endif

    /* 等待发送完毕 */
    if (pdTRUE == xSemaphoreTake(uart_data->xTxSem, (TickType_t)timeout_ms))
    {
        xSemaphoreGive(uart_data->xTxMutex);
        return 0;
    }

    /* 超时：中止本次发送 */
#if UART_DMA_MODE
    usart_dma_transmit_config(USART_UX, USART_TRANSMIT_DMA_DISABLE);
    dma_channel_disable(USART_DMA, USART_DMA_TX_CH);
#endif
#if UART_INTERRUPT_MODE
    usart_interrupt_disable(USART_UX, USART_INT_TBE);
#endif
    usart_interrupt_disable(USART_UX, USART_INT_TC);
    xSemaphoreTake(uart_data->xTxSem, 0);
    xSemaphoreGive(uart_data->xTxMutex);
    return -1;
#endif
}

static int gd32_uart_recv(struct UART_Device *pDev, uint8_t *data, int timeout_ms)
{
    struct UART_Data *uart_data = pDev->priv_data;

#if !UART_DELAY_MODE
    /* 中断里写队列，这里从队列取数据；timeout_ms 直接作为 tick 使用 */
    if (pdPASS == xQueueReceive(uart_data->xRxQueue, data, (TickType_t)timeout_ms))
    {
        return 0;
    }
    return -1;
#else
    while (timeout_ms-- > 0)
    {
        if (usart_flag_get(USART_UX, USART_FLAG_RBNE) != RESET)
        {
            *data = (uint8_t)usart_data_receive(USART_UX);
            return 0;
        }
        vTaskDelay(1);
    }
    return -1;
#endif
}


static struct UART_Device gd32_uart0 = {
    "gd32_uart0",
    gd32_uart_init,
    gd32_uart_send,
    gd32_uart_recv,
    &g_gd32_uart0_data
};

static struct UART_Device *g_uart_devs[] = {
    &gd32_uart0
};

struct UART_Device *GetUARTDevice(char *name)
{
    for (uint8_t i = 0; i < sizeof(g_uart_devs) / sizeof(g_uart_devs[0]); i++)
    {
        if (0 == strcmp(name, g_uart_devs[i]->name))
        {
            return g_uart_devs[i];
        }
    }

    return NULL;
}
