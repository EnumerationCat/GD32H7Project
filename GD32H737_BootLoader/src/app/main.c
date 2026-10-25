
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

extern volatile uint32_t g_timer_isr_count;
extern volatile unsigned long ulHighFrequencyTimerTicks;
volatile uint32_t g_systick_count;

/* 向量表就留在 flash 里：链接地址 0x08010400（见 gd32h7xx_flash.ld），
   VTOR 由 SystemInit 按 VECT_TAB_OFFSET 设置，bootloader 跳转前也设成同一个值。
   注意：不能只做 128/512 字节对齐。ARMv7-M 要求按"异常个数向上取整到 2 的幂"对齐
   （本表 0x3A4 字节 -> 1024 字节）；否则外部中断取向量会落到错误的表项，
   186 号 IRQ 会取到 0 -> PC=0 -> INVSTATE -> HardFault，
   而 SysTick/HardFault 这类系统异常不受影响，表现很有迷惑性。 */

int main(void)
{
    delay_init(600);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */
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

	g_systick_count++;

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

	printf("[DBG] tasks created\r\n");

	/*启动任务执行一次，删除启动任务*/
	vTaskDelete(start_TaskHandle);

	taskEXIT_CRITICAL(); // 退出临界区，允许任务切换


}


void task1(void *pvParameters) {

	printf("[DBG] task1 lv_init...\r\n");
	lv_init();
	printf("[DBG] task1 disp init...\r\n");
	lv_port_disp_init();
	printf("[DBG] task1 indev init...\r\n");
	lv_port_indev_init();
	printf("[DBG] task1 ui...\r\n");
	user_home();
	printf("[DBG] task1 ui ok\r\n");
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
	printf("[DBG] task2 start\r\n");

    while (1) {
       // printf("任务2:....\r\n");

		LED1_TOGGLE();
		printf("[DBG] task2 tick\r\n");

        vTaskDelay(1000);
    }
}


/* [DBG] 打印硬错误现场，定位跑飞位置 */
void hardfault_report(uint32_t *sp, uint32_t exc_return)
{
	const uint32_t *base = sp - 32;     /* 从 sp-0x80 开始，覆盖可能位于下方的定时器异常帧 */

	printf("\r\n[DBG] HARDFAULT exc_return=0x%08lX\r\n", (unsigned long)exc_return);
	printf("[DBG] sp=0x%08lX msp=0x%08lX psp=0x%08lX control=0x%08lX primask=%lu ccr=0x%08lX\r\n",
	       (unsigned long)sp, (unsigned long)__get_MSP(), (unsigned long)__get_PSP(),
	       (unsigned long)__get_CONTROL(), (unsigned long)__get_PRIMASK(), (unsigned long)SCB->CCR);
	/* Cortex-M7: 基本栈帧就在 handler 的 sp 处，FP 上下文(S0-S15/FPSCR)在它上方 */
	printf("[DBG] pc=0x%08lX lr=0x%08lX xpsr=0x%08lX\r\n",
	       (unsigned long)sp[6], (unsigned long)sp[5], (unsigned long)sp[7]);
	printf("[DBG] r0=0x%08lX r1=0x%08lX r2=0x%08lX r3=0x%08lX r12=0x%08lX\r\n",
	       (unsigned long)sp[0], (unsigned long)sp[1], (unsigned long)sp[2],
	       (unsigned long)sp[3], (unsigned long)sp[4]);
	printf("[DBG] cfsr=0x%08lX hfsr=0x%08lX mmfar=0x%08lX bfar=0x%08lX\r\n",
	       (unsigned long)SCB->CFSR, (unsigned long)SCB->HFSR,
	       (unsigned long)SCB->MMFAR, (unsigned long)SCB->BFAR);
	printf("[DBG] icsr=0x%08lX vtor=0x%08lX vec186=0x%08lX\r\n",
	       (unsigned long)SCB->ICSR, (unsigned long)SCB->VTOR,
	       (unsigned long)((volatile uint32_t *)(uintptr_t)SCB->VTOR)[186]);
	printf("[DBG] mpu_ctrl=0x%08lX shcsr=0x%08lX\r\n",
	       (unsigned long)MPU->CTRL, (unsigned long)SCB->SHCSR);
	printf("[DBG] isr_cnt=%lu ticks=%lu systick=%lu fpccr=0x%08lX fcar=0x%08lX cacr=0x%08lX ccsidr=0x%08lX\r\n",
	       (unsigned long)g_timer_isr_count, (unsigned long)ulHighFrequencyTimerTicks,
	       (unsigned long)g_systick_count,
	       (unsigned long)FPU->FPCCR, (unsigned long)FPU->FPCAR,
	       (unsigned long)SCB->CACR, (unsigned long)SCB->CCSIDR);
	/* 被打断的那个异常(带 FP 扩展帧)的基本栈帧：FPCAR 指向 FP 区，基本帧在它下面 0x20 */
	if(0U != FPU->FPCAR) {
		const uint32_t *tf = (const uint32_t *)((uintptr_t)FPU->FPCAR - 0x20U);

		printf("[DBG] fpframe@0x%08lX: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\r\n",
		       (unsigned long)(uintptr_t)tf,
		       (unsigned long)tf[0], (unsigned long)tf[1], (unsigned long)tf[2],
		       (unsigned long)tf[3], (unsigned long)tf[4], (unsigned long)tf[5],
		       (unsigned long)tf[6], (unsigned long)tf[7]);
	}
	/* 原始内存，带地址打印，方便核对帧到底落在哪 */
	for (int32_t i = 0; i < 44; i += 4) {
		printf("[DBG] %08lX: %08lX %08lX %08lX %08lX\r\n",
		       (unsigned long)(uintptr_t)&base[i],
		       (unsigned long)base[i + 0], (unsigned long)base[i + 1],
		       (unsigned long)base[i + 2], (unsigned long)base[i + 3]);
	}
	for (;;) {
	}
}

__attribute__((naked)) void HardFault_Handler(void)
{
	__asm volatile(
		"tst lr, #4\n\t"
		"ite eq\n\t"
		"mrseq r0, msp\n\t"
		"mrsne r0, psp\n\t"
		"mov r1, lr\n\t"
		"b hardfault_report\n\t");
}


