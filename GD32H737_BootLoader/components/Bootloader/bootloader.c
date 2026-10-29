#include "bootloader.h"
#include "gd32h7xx.h"
#include "gd32h7xx_fmc.h"




typedef unsigned int __be32;

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN	32	/* Image Name Length		*/

typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;






/* 调试串口：与 App 保持一致（USART0 / PA9 TX / 115200-8-N-1），方便同一终端观察 */
#define BOOT_UART           USART0
#define BOOT_UART_CLK       RCU_USART0
#define BOOT_UART_TX_PORT   GPIOA
#define BOOT_UART_TX_PIN    GPIO_PIN_9
#define BOOT_UART_TX_AF     GPIO_AF_7
#define BOOT_UART_TX_CLK    RCU_GPIOA
#define BOOT_UART_RX_PORT   GPIOA
#define BOOT_UART_RX_PIN    GPIO_PIN_10
#define BOOT_UART_RX_AF     GPIO_AF_7
#define BOOT_UART_RX_CLK    RCU_GPIOA
#define BOOT_UART_BAUD      115200U

static void boot_uart_init(void)
{
    rcu_periph_clock_enable(BOOT_UART_TX_CLK);
    rcu_periph_clock_enable(BOOT_UART_RX_CLK);
    rcu_periph_clock_enable(BOOT_UART_CLK);

    gpio_af_set(BOOT_UART_TX_PORT, BOOT_UART_TX_AF, BOOT_UART_TX_PIN);
    gpio_mode_set(BOOT_UART_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BOOT_UART_TX_PIN);
    gpio_output_options_set(BOOT_UART_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, BOOT_UART_TX_PIN);

    gpio_af_set(BOOT_UART_RX_PORT, BOOT_UART_RX_AF, BOOT_UART_RX_PIN);
    gpio_mode_set(BOOT_UART_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BOOT_UART_RX_PIN);

    usart_deinit(BOOT_UART);
    usart_baudrate_set(BOOT_UART, BOOT_UART_BAUD);
    usart_stop_bit_set(BOOT_UART, USART_STB_1BIT);
    usart_word_length_set(BOOT_UART, USART_WL_8BIT);
    usart_parity_config(BOOT_UART, USART_PM_NONE);
    usart_transmit_config(BOOT_UART, USART_TRANSMIT_ENABLE);
    usart_receive_config(BOOT_UART, USART_RECEIVE_ENABLE);
    usart_enable(BOOT_UART);
}

void boot_putc(uint8_t c)
{
    while(RESET == usart_flag_get(BOOT_UART, USART_FLAG_TC)) {
    }
    usart_data_transmit(BOOT_UART, c);
}

void boot_puts(const char *s)
{
    while('\0' != *s) {
        boot_putc((uint8_t)*s++);
    }
}

/* 非阻塞读：已收到字节返回 0~255，无数据返回 -1 */
int boot_getc(void)
{
    if(RESET == usart_flag_get(BOOT_UART, USART_FLAG_RBNE)) {
        return -1;
    }
    return (int)usart_data_receive(BOOT_UART);
}

/* 等最后一个字节真正移出，避免跳转后 App 重配串口把它掐掉 */
static void boot_uart_flush(void)
{
    while(RESET == usart_flag_get(BOOT_UART, USART_FLAG_TC)) {
    }
}

static void boot_puthex32(uint32_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    boot_puts("0x");
    for(int i = 28; i >= 0; i -= 4) {
        boot_putc((uint8_t)hex[(v >> i) & 0xFU]);
    }
}

/* uImage 头按大端存储，读入后需要转换 */
static uint32_t ih_be32(__be32 v)
{
    return __builtin_bswap32((uint32_t)v);
}

int bootloader_app_is_valid(uint32_t image_base, uint32_t *app_base)
{
    const image_header_t *hdr = (const image_header_t *)image_base;
    uint32_t load, ep, size, sp, pc;

    if(IH_MAGIC != ih_be32(hdr->ih_magic)) {
        return 0;
    }

    load = ih_be32(hdr->ih_load);//app start 的位置
    ep   = ih_be32(hdr->ih_ep);  //app reset_handler的位置
    size = ih_be32(hdr->ih_size);//app的大小

    /* 数据须落在 App 分区内，且不能覆盖 uImage 头本身 */
    if((load < (APP_BASE_ADDR + sizeof(image_header_t))) || (load >= FLASH_END_ADDR)) {
        return 0;
    }
    if(size > (FLASH_END_ADDR - load)) {
        return 0;
    }

    /* ARMv7-M：向量表必须按"异常个数向上取整到 2 的幂"对齐。
       App 的表 0x3A4 字节 -> 1024 字节（不是 128！只做 128 对齐时，
       外部中断取向量会落到错误的表项，186 号 IRQ 会取到 0 -> HardFault） */
    if(0U != (load & 0x3FFU)) {
        return 0;
    }

    /* 入口点须在 App 的 Flash 区间内，且末位为 1（Thumb 位） */
    if((ep < load) || (ep >= FLASH_END_ADDR)) {
        return 0;
    }
    if(0U == (ep & 1U)) {
        return 0;
    }

    sp = *(volatile uint32_t *)load;
    pc = *(volatile uint32_t *)(load + 4U);

    /* 栈顶必须落在 App 使用的 RAM 区间内 */
    if((sp < APP_SRAM_START) || (sp > APP_SRAM_END)) {
        return 0;
    }

    /* 复位向量必须落在 App 的 Flash 区间内，且末位为 1（Thumb 位） */
    if((pc < load) || (pc >= FLASH_END_ADDR)) {
        return 0;
    }
    if(0U == (pc & 1U)) {
        return 0;
    }

    *app_base = load;
    return 1;
}

void bootloader_jump_to_app(uint32_t app_base)
{
    /* 先取出 App 的栈顶与复位向量，之后不再使用本函数的栈 */
    uint32_t sp = *(volatile uint32_t *)app_base;
    uint32_t pc = *(volatile uint32_t *)(app_base + 4U);

    /* 关中断，清 SysTick，避免残留中断进 App */
    __disable_irq();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;

    /* 关闭并清除所有 NVIC 中断 */
    for(uint32_t i = 0U; i < 8U; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    /* 把向量表重定向到 App */
    SCB->VTOR = app_base;
    __DSB();
    __ISB();

    /* 设置 App 栈顶后跳转（bx 保证不再访存本栈）。
       最后恢复 PRIMASK：App 按复位后的默认状态运行，FreeRTOS 依赖中断启动调度器 */
    __set_MSP(sp);
    __enable_irq();
    __ASM volatile("bx %0" : : "r"(pc));

    for(;;) {
    }
}

/* 共享 RAM 的 ITCM/DTCM 划分由选项字节决定（出厂默认 ITCM 64K + DTCM 128K）。
   链接脚本按 256K/256K 布局，故首次运行需改写选项字节并复位一次使其生效。 */
static void boot_tcm_shared_ram_ensure(void)
{
    uint32_t itcm_kb = 0U;
    uint32_t dtcm_kb = 0U;
    fmc_state_enum st;

    ob_tcm_shared_ram_size_get(&itcm_kb, &dtcm_kb);
    if((256U == itcm_kb) && (256U == dtcm_kb)) {
        return;
    }

    boot_puts("[BOOT] TCM shared RAM not 256K/256K, reconfiguring and resetting...\r\n");
    boot_uart_flush();

    ob_unlock();
    ob_tcm_shared_ram_config(OB_ITCM_SHARED_RAM_256KB, OB_DTCM_SHARED_RAM_256KB);
    st = ob_start();
    ob_lock();

    /* 只写失败时继续运行，避免反复写 flash 造成复位死循环 */
    if(FMC_READY != st) {
        boot_puts("[BOOT] TCM option byte write FAILED, keep default\r\n");
        return;
    }

    NVIC_SystemReset();     /* 选项字节复位后生效，不返回 */
    for(;;) {
    }
}

int main(void)
{
    uint32_t app_base = 0U;

    /* SystemInit() 已由 Reset_Handler 调用：时钟就绪，VTOR = 0x08000000 */
    boot_uart_init();

    boot_puts("\r\n[BOOT] bootloader running\r\n");

    boot_tcm_shared_ram_ensure();

    if(bootloader_app_is_valid(APP_BASE_ADDR, &app_base)) {
        boot_puts("[BOOT] uImage valid  load=");
        boot_puthex32(app_base);
        boot_puts(" msp=");
        boot_puthex32(*(volatile uint32_t *)app_base);
        boot_puts(" reset_handle=");
        boot_puthex32(*(volatile uint32_t *)(app_base + 4U));
        boot_puts(" -> jumping\r\n");
        boot_uart_flush();

        bootloader_jump_to_app(app_base);
    }

    boot_puts("[BOOT] no valid uImage, stay in bootloader\r\n");
    for(;;) {
    }
}
