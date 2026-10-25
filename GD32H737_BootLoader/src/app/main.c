
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


int main(void)
{


    sys_cache_enable();                     /* 使能CPU cache */
    delay_init(600);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);
    led_init();							                /* 初始化LED */   
	norflash_init();
    mpu_memory_protection();                /* 保护相关存储区域 */


	
	timerx_int_init(10 - 1, 30000 - 1);   /* 300 000 000 / 30000 = 10KHz 10KHz的计数频率，计数10次为1ms */


	//delay_ms(500);
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


