
#include "lv_obj.h"
#include "lv_timer.h"
#include "sys.h"
#include "led.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>
#include "mpu.h"
#include "timer.h"
#include "lcd.h"
#include "touch.h"

#include "FreeRTOS.h"
#include "task.h"


#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "norflash.h"
// #include "lv_demo_widgets.h"
// #include "lv_demo_stress.h"
// #include "lv_demo_music.h"
// #include "lv_demo_keypad_encoder.h"
// #include "lv_demo_benchmark.h"
#include "user_home.h"
#include "gd32h7xx_misc.h"

//#include "timer.h"

void freertos_start(void);


/* 向量表就留在 flash 里：链接地址 0x08010400（见 gd32h7xx_flash.ld），
   VTOR 由 SystemInit 按 VECT_TAB_OFFSET 设置，bootloader 跳转前也设成同一个值。
   注意：不能只做 128/512 字节对齐。ARMv7-M 要求按"异常个数向上取整到 2 的幂"对齐
   （本表 0x3A4 字节 -> 1024 字节）；否则外部中断取向量会落到错误的表项，
   186 号 IRQ 会取到 0 -> PC=0 -> INVSTATE -> HardFault，
   而 SysTick/HardFault 这类系统异常不受影响，表现很有迷惑性。 */

int main(void)
{
    /* FreeRTOS 要求所有抢占优先级位都用作抢占位。SPL 的 nvic_irq_enable 在分组仍是
       复位值时会自动改成 PRE2_SUB2，之后 FreeRTOS 的 FromISR 优先级校验断言失败
       （configASSERT 是关中断死循环），必须在任何 nvic_irq_enable 之前设置好 */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    delay_init(600);                        /* 延时初始化 */

    /* 串口统一走 UART_Device：Init 内部完成 GPIO/USART/DMA/中断配置 */
    struct UART_Device *uart_dev = GetUARTDevice("gd32_uart0");
    if (NULL != uart_dev)
    {
        uart_dev->Init(uart_dev, 115200, 8, 'N', 1);
    }
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);
    led_init();							                /* 初始化LED */
	norflash_init();
    mpu_memory_protection();                /* 先把内存属性定下来 */
    sys_cache_enable();                     /* 再开 cache：顺序不能反，否则栈会被脏行回写覆盖 */

	timerx_int_init(10 - 1, 30000 - 1);   /* 300 000 000 / 30000 = 10KHz 10KHz的计数频率，计数10次为1ms */

	freertos_start();



	

    while (1)
    {        

    }
}



#if SYS_SUPPORT_OS

/**
 * @brief 栈溢出钩子函数（configCHECK_FOR_STACK_OVERFLOW>0 时必须实现）
 * @param pxTask 发生栈溢出的任务句柄
 * @param pcTaskName 发生栈溢出的任务名
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    /* 栈溢出处理逻辑，比如打印任务名、触发硬件看门狗、记录日志等 */
    ( void ) pxTask;
    ( void ) pcTaskName;
    /* 死循环（可选，便于调试定位问题） */
    for( ;; )
	{
		
		vTaskDelay(1000); // 延时1秒，避免打印过快导致串口堵塞
	}
}



 
extern void xPortSysTickHandler(void);

/**
 * @brief       systick中断服务函数,使用OS时用到
 * @param       ticks: 延时的节拍数
 * @retval      无
 */



void SysTick_Handler(void)
{

	//LED0(0);


	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		xPortSysTickHandler();
	}
}

#endif




/*启动任务的配置*/
#define START_TASK_STACK 130
#define START_TASK_PRIORITY 1
TaskHandle_t start_TaskHandle;
void start_Task(void * pvParameters);


/*任务1的配置*/
#define TASK1_STACK 1024
#define TASK1_PRIORITY 2
TaskHandle_t task1_TaskHandle;
void task1(void * pvParameters);


/*任务2的配置*/
#define TASK2_STACK 130
#define TASK2_PRIORITY 3
TaskHandle_t task2_TaskHandle;
void task2(void * pvParameters);

/*串口实验任务的配置*/
#define UART_TASK_STACK 2048
#define UART_TASK_PRIORITY 4
TaskHandle_t uart_taskHandle;
void uart_task(void * pvParameters);






void freertos_start(void)
{

	
	/*创建启动任务*/
	xTaskCreate((TaskFunction_t) start_Task,//任务函数的地址
		 (char *) "start_Task",//任务名字
		 (configSTACK_DEPTH_TYPE) START_TASK_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) START_TASK_PRIORITY,//任务优先级
		 (TaskHandle_t *) &start_TaskHandle//任务句柄的地址
		);

	
	/*启动调度器*/
	vTaskStartScheduler();

	

}



void start_Task(void * pvParameters)
{
	printf("[DBG] start_task\r\n");

	taskENTER_CRITICAL(); // 进入临界区，防止任务切换干扰任务创建过程
	/*创建启动任务*/
	xTaskCreate((TaskFunction_t) task1,//任务函数的地址
		 (char *) "task1",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK1_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK1_PRIORITY,//任务优先级
		 (TaskHandle_t *) &task1_TaskHandle//任务句柄的地址

		);

	
	xTaskCreate((TaskFunction_t) task2,//任务函数的地址
		 (char *) "task2",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK2_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK2_PRIORITY,//任务优先级
		 (TaskHandle_t *) &task2_TaskHandle//任务句柄的地址

		);

	xTaskCreate((TaskFunction_t) uart_task,//任务函数的地址
		 (char *) "uart_task",//任务名字
		 (configSTACK_DEPTH_TYPE) UART_TASK_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) UART_TASK_PRIORITY,//任务优先级
		 (TaskHandle_t *) &uart_taskHandle//任务句柄的地址

		);

	printf("[DBG] tasks created\r\n");

	/*启动任务执行一次，删除启动任务*/
	vTaskDelete(start_TaskHandle);

	taskEXIT_CRITICAL(); // 退出临界区，允许任务切换


}


void task1(void *pvParameters) {

	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	user_home();

	//lv_demo_widgets();
	//lv_demo_stress();
	//lv_demo_benchmark();



    while (1) {

		tp_dev.scan(0);	            /*触摸扫描*/
		
        //printf("任务1:LVGL...\r\n");
        

		lv_timer_handler(); // 处理LVGL定时器事件



        // 打印栈和堆信息
        // UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        // printf("任务1 最小剩余栈: %u 字, 堆空闲: %u 字节, 历史最小: %u 字节\r\n",
        //           stack, xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
        vTaskDelay(5);
    }
}



void task2(void * pvParameters)
{
    while (1) {
       // printf("任务2:....\r\n");

		LED1_TOGGLE();

        vTaskDelay(1000);
    }
}


/* 串口实验任务：启动时发一串提示，之后收到的字节原样回显 */
void uart_task(void * pvParameters)
{
    static const char banner[] = "uart task ready, echo mode\r\n";
    struct UART_Device *uart_dev = GetUARTDevice("gd32_uart0");
    uint8_t data;

    if (NULL == uart_dev)
    {
        vTaskDelete(NULL);
    }

    uart_dev->Send(uart_dev, (uint8_t *)banner, sizeof(banner) - 1, 100);

    while (1)
    {
		
        /* Recv 超时单位是 tick；收到一个字节就发回去 */
        if (0 == uart_dev->Recv(uart_dev, &data, 100))
        {
            uart_dev->Send(uart_dev, &data, 1, 100);
        }
		vTaskDelay(1);
    }
}


