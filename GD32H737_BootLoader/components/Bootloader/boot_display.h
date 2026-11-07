#ifndef __BOOT_DISPLAY_H__
#define __BOOT_DISPLAY_H__

#include <stdint.h>

/* 初始化 LCD 并铺成黑底（和 App 的 shell 页面一致，不带关闭按钮） */
void boot_display_init(void);

/* 串口每输出一个字符时调用，按终端语义缓存到屏幕日志 */
void boot_display_putc(char c);

/* 主循环空闲时调用，有变化才重绘（满屏黑底 + 白色文本，自动滚动） */
void boot_display_poll(void);

/* ZMODEM 传输期间屏蔽镜像，避免协议二进制被当成文本显示 */
void boot_display_suspend(int suspend);

/* 传输期间更新屏幕状态行：只画 LCD、不写串口，也不受 suspend 影响。
   传入不带 "\r" 的一行文本，会覆盖当前行；结束后调用 _end 换行。 */
void boot_display_progress(const char *s);
void boot_display_progress_end(void);

/* lcd 驱动需要的基础延时（bootloader 不跑 FreeRTOS，自己用 SysTick 实现） */
void delay_ms(uint16_t nms);

#endif
