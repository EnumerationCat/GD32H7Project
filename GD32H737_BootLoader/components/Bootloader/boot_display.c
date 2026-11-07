#include <string.h>

#include "boot_display.h"
#include "lcd.h"
#include "gd32h7xx.h"

#define BOOT_DISP_FONT      16U                             /* 8x16 点阵 */
#define BOOT_DISP_COLS_MAX  64U
#define BOOT_DISP_SIZE      2048U

/* 屏幕日志：只保存文本，主循环空闲时再统一重绘 */
#define BOOT_DISP_DIRTY_NONE    0
#define BOOT_DISP_DIRTY_LAST    1       /* 只有最后一行变了 */
#define BOOT_DISP_DIRTY_ALL     2       /* 行数/内容整体变化，需要整屏重画 */

static char s_disp[BOOT_DISP_SIZE + 1U];
static uint32_t s_disp_len = 0U;
static uint32_t s_disp_seen_total = 0U;  /* 已画到屏幕上的行数（上次 poll 时的总行数） */
static int s_disp_ready = 0;
static int s_disp_dirty = BOOT_DISP_DIRTY_NONE;
static int s_disp_suspended = 0;
static int s_pending_cr = 0;

/* lcd.c 里的 16x8 ASCII 字库；只引用 16 号，其余字库由 --gc-sections 裁掉 */
extern const unsigned char asc2_1608[95][16];

/* EXMC(0x60000000) 默认落在 Cortex-M7 的 Normal 内存区，写缓冲会把对同一
   地址（LCD_RAM）的连续写合并/重排，清屏和画点会大量丢写，表现为只清出一
   小块、旧画面还在。App 由 mpu_memory_protection() 把该区域配成 Device
   属性；bootloader 没有 MPU 配置，这里补一条 Strongly-ordered 的区域
   （和 App 的 mpu.c 一致），其余地址走默认映射。 */
#define BOOT_DISP_MPU_REGION    4U
#define BOOT_DISP_EXMC_BASE     0x60000000U
#define BOOT_DISP_EXMC_SIZE     25U     /* 64MB -> 2^(SIZE+1) */

static void boot_disp_mpu_lcd(void)
{
    ARM_MPU_Disable();
    ARM_MPU_SetRegionEx(BOOT_DISP_MPU_REGION, BOOT_DISP_EXMC_BASE,
                        MPU_RASR_ENABLE_Msk |
                        (BOOT_DISP_EXMC_SIZE << MPU_RASR_SIZE_Pos) |
                        (ARM_MPU_AP_FULL << MPU_RASR_AP_Pos));  /* TEX/C/B/S=0 */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}

static void boot_disp_char16(uint16_t x, uint16_t y, char chr, uint16_t color)
{
    const unsigned char *pfont;
    uint16_t y0 = y;
    uint8_t t;
    uint8_t bit;
    uint8_t temp;

    if((chr < ' ') || (chr > '~')) {
        return;
    }
    pfont = asc2_1608[(uint8_t)chr - ' '];

    for(t = 0U; t < BOOT_DISP_FONT; t++) {
        temp = pfont[t];
        for(bit = 0U; bit < 8U; bit++) {
            if(0U != (temp & 0x80U)) {
                lcd_draw_point(x, y, color);
            }
            temp <<= 1;
            y++;

            /* 先判断这一列是否画完：最后一行 y0 = height-16，画完第 16 个
               点时 y 正好等于 height，若先判断越界会丢掉这一列之后的笔画 */
            if((uint16_t)(y - y0) == BOOT_DISP_FONT) {
                y = y0;
                x++;
                if(x >= lcddev.width) {
                    return;
                }
                break;
            }
            if(y >= lcddev.height) {
                return;
            }
        }
    }
}

static void boot_disp_string(uint16_t x, uint16_t y, const char *s)
{
    while(('\0' != *s) && (*s >= ' ') && (*s <= '~')) {
        boot_disp_char16(x, y, *s, WHITE);
        x = (uint16_t)(x + (BOOT_DISP_FONT / 2U));
        if(x >= lcddev.width) {
            break;
        }
        s++;
    }
}

/* lcd.c / lcd_ex.c 里的复位等待用它；SysTick 轮询计时，不开中断 */
void delay_ms(uint16_t nms)
{
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    for(uint32_t i = 0U; i < nms; i++) {
        while(0U == (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {
        }
    }

    SysTick->CTRL = 0U;
}

static void boot_disp_mark(int level)
{
    if(level > s_disp_dirty) {
        s_disp_dirty = level;
    }
}

static uint32_t boot_disp_line_count(void)
{
    uint32_t total = 1U;

    for(uint32_t i = 0U; i < s_disp_len; i++) {
        if('\n' == s_disp[i]) {
            total++;
        }
    }
    return total;
}

/* 缓冲满时丢掉最前面的一部分，尽量从换行处分界 */
static void boot_disp_trim(void)
{
    uint32_t cut = s_disp_len / 2U;

    while((cut < s_disp_len) && ('\n' != s_disp[cut])) {
        cut++;
    }
    if(cut >= s_disp_len) {
        s_disp_len = 0U;
        boot_disp_mark(BOOT_DISP_DIRTY_ALL);
        return;
    }

    cut++;      /* 保留换行之后的内容 */
    memmove(s_disp, s_disp + cut, s_disp_len - cut);
    s_disp_len -= cut;
    boot_disp_mark(BOOT_DISP_DIRTY_ALL);
}

static void boot_disp_putc_raw(char c)
{
    if(s_disp_len >= (BOOT_DISP_SIZE - 1U)) {
        boot_disp_trim();
    }

    s_disp[s_disp_len++] = c;
    s_disp[s_disp_len] = '\0';
    boot_disp_mark(BOOT_DISP_DIRTY_LAST);
}

/* 模拟终端：\r 回到行首（后续字符覆盖本行），\r 紧跟 \n 视为普通换行 */
void boot_display_putc(char c)
{
    if((0 == s_disp_ready) || (0 != s_disp_suspended)) {
        return;
    }

    if('\r' == c) {
        s_pending_cr = 1;
        return;
    }

    if('\n' == c) {
        s_pending_cr = 0;
        boot_disp_putc_raw('\n');
        /* 行数超过屏幕可见行后需要整体上滚，只能整屏重画 */
        if(boot_disp_line_count() > (lcddev.height / BOOT_DISP_FONT)) {
            boot_disp_mark(BOOT_DISP_DIRTY_ALL);
        }
        return;
    }

    if(s_pending_cr) {
        s_pending_cr = 0;
        /* 光标回到行首：删掉当前行已显示的内容 */
        while((s_disp_len > 0U) && ('\n' != s_disp[s_disp_len - 1U])) {
            s_disp_len--;
        }
        boot_disp_mark(BOOT_DISP_DIRTY_LAST);
    }

    if('\b' == c) {
        if((s_disp_len > 0U) && ('\n' != s_disp[s_disp_len - 1U])) {
            s_disp_len--;
            boot_disp_mark(BOOT_DISP_DIRTY_LAST);
        }
        return;
    }

    if((c < 0x20) || (c > 0x7E)) {
        return;
    }

    boot_disp_putc_raw(c);
}

static void boot_disp_render(void)
{
    uint32_t total = 1U;
    uint32_t skip;
    uint32_t p;
    uint32_t ln;
    uint32_t cols = lcddev.width / (BOOT_DISP_FONT / 2U);
    uint16_t y;
    char line[BOOT_DISP_COLS_MAX + 1U];

    for(uint32_t i = 0U; i < s_disp_len; i++) {
        if('\n' == s_disp[i]) {
            total++;
        }
    }

    skip = (total > (lcddev.height / BOOT_DISP_FONT)) ?
           (total - (lcddev.height / BOOT_DISP_FONT)) : 0U;

    /* 跳过滚出屏幕的旧行 */
    p = 0U;
    for(ln = 0U; (ln < skip) && (p < s_disp_len); p++) {
        if('\n' == s_disp[p]) {
            ln++;
        }
    }

    /* 逐行擦黑再画字：不用整屏 lcd_clear，避免上滚时先闪一帧全黑 */
    y = 0U;
    while((p < s_disp_len) && (((uint32_t)y + BOOT_DISP_FONT) <= lcddev.height)) {
        uint32_t n = 0U;

        while((p < s_disp_len) && ('\n' != s_disp[p])) {
            if((n < cols) && (n < (BOOT_DISP_COLS_MAX - 1U))) {
                line[n++] = s_disp[p];
            }
            p++;
        }
        line[n] = '\0';

        lcd_fill(0U, y, (uint16_t)(lcddev.width - 1U),
                 (uint16_t)(y + BOOT_DISP_FONT - 1U), BLACK);
        boot_disp_string(0U, y, line);

        y += BOOT_DISP_FONT;
        if((p < s_disp_len) && ('\n' == s_disp[p])) {
            p++;
        }
    }

    /* 剩余行擦黑，防止 trim 后旧文本残留 */
    while(((uint32_t)y + BOOT_DISP_FONT) <= lcddev.height) {
        lcd_fill(0U, y, (uint16_t)(lcddev.width - 1U),
                 (uint16_t)(y + BOOT_DISP_FONT - 1U), BLACK);
        y += BOOT_DISP_FONT;
    }
}

/* 取出缓冲里第 index 行的文本（截断到屏幕列宽） */
static int boot_disp_get_line(uint32_t index, char *out, uint32_t cols)
{
    uint32_t p = 0U;
    uint32_t ln = 0U;
    uint32_t n = 0U;

    while((p < s_disp_len) && (ln < index)) {
        if('\n' == s_disp[p]) {
            ln++;
        }
        p++;
    }
    if(ln != index) {
        return 0;
    }

    while((p < s_disp_len) && ('\n' != s_disp[p])) {
        if(n < cols) {
            out[n++] = s_disp[p];
        }
        p++;
    }
    out[n] = '\0';
    return 1;
}

/* 重画一整行：先擦掉该行再画字，避免旧笔画残留 */
static void boot_disp_draw_line(uint32_t index, uint16_t y)
{
    uint32_t cols = lcddev.width / (BOOT_DISP_FONT / 2U);
    char line[BOOT_DISP_COLS_MAX + 1U];

    if(cols > (BOOT_DISP_COLS_MAX - 1U)) {
        cols = BOOT_DISP_COLS_MAX - 1U;
    }
    if(0 == boot_disp_get_line(index, line, cols)) {
        return;
    }

    lcd_fill(0U, y, (uint16_t)(lcddev.width - 1U),
             (uint16_t)(y + BOOT_DISP_FONT - 1U), BLACK);
    boot_disp_string(0U, y, line);
}

void boot_display_poll(void)
{
    uint32_t total;
    uint32_t max_lines;

    if((0 == s_disp_ready) || (BOOT_DISP_DIRTY_NONE == s_disp_dirty)) {
        return;
    }

    total = boot_disp_line_count();
    max_lines = lcddev.height / BOOT_DISP_FONT;

    /* 整屏重画：缓冲 trim 丢行，或超过屏幕可见行需要整体上滚 */
    if((BOOT_DISP_DIRTY_ALL == s_disp_dirty) || (total < s_disp_seen_total)) {
        s_disp_dirty = BOOT_DISP_DIRTY_NONE;
        boot_disp_render();
        s_disp_seen_total = total;
        return;
    }

    s_disp_dirty = BOOT_DISP_DIRTY_NONE;

    if(total > max_lines) {
        /* 滚屏后屏幕固定显示最后 max_lines 行；没有新增行时只重画最后一行 */
        if(total != s_disp_seen_total) {
            boot_disp_render();
            s_disp_seen_total = total;
        } else {
            boot_disp_draw_line(total - 1U,
                                (uint16_t)((max_lines - 1U) * BOOT_DISP_FONT));
        }
        return;
    }

    /* 一次 poll 之间可能追加了多行（命令输出），逐行补画，不能只画最后一行。
       旧内容以 '\n' 结尾时，缓冲末尾已经算出一行空的“最后一行”，新输出先填这
       一行再靠新的 '\n' 才让行数增加；只从 seen_total 开始画会漏掉被填满的
       那一行（表现为最后几行不显示），所以回退一行重画。 */
    if(total > s_disp_seen_total) {
        uint32_t first = (s_disp_seen_total > 0U) ? (s_disp_seen_total - 1U) : 0U;

        while(first < total) {
            boot_disp_draw_line(first, (uint16_t)(first * BOOT_DISP_FONT));
            first++;
        }
        s_disp_seen_total = total;
        return;
    }

    /* 已在屏幕上的最后一行内容变了：输入/退格/\r 覆盖 */
    if(total > 0U) {
        boot_disp_draw_line(total - 1U,
                            (uint16_t)((total - 1U) * BOOT_DISP_FONT));
    }
}

void boot_display_suspend(int suspend)
{
    s_disp_suspended = suspend;
}

/* 传输期间的状态行：绕开 suspend 只写屏幕日志，并立即刷新。
   用 "\r" 覆盖当前行，所以反复调用不会在日志里堆出多行。 */
void boot_display_progress(const char *s)
{
    int saved;

    if(0 == s_disp_ready) {
        return;
    }

    saved = s_disp_suspended;
    s_disp_suspended = 0;
    boot_display_putc('\r');
    while('\0' != *s) {
        boot_display_putc(*s++);
    }
    s_disp_suspended = saved;

    boot_display_poll();
}

/* 状态行结束：补一个换行，后面的日志另起一行 */
void boot_display_progress_end(void)
{
    int saved;

    if((0 == s_disp_ready) || (0U == s_disp_len) ||
       ('\n' == s_disp[s_disp_len - 1U])) {
        return;
    }

    saved = s_disp_suspended;
    s_disp_suspended = 0;
    boot_display_putc('\n');
    s_disp_suspended = saved;
}

void boot_display_init(void)
{
    boot_disp_mpu_lcd();    /* 必须在第一次写 LCD 之前完成 */
    lcd_init();             /* 内部会先清成白屏 */
    lcd_display_dir(1);     /* 与 App 一致：横屏 480x320 */
    g_back_color = BLACK;   /* 字符空白点也画黑底 */
    lcd_clear(BLACK);       /* 黑底，不带关闭按钮 */

    s_disp_len = 0U;
    s_disp[0] = '\0';
    s_disp_seen_total = 0U;
    s_pending_cr = 0;
    s_disp_dirty = BOOT_DISP_DIRTY_NONE;
    s_disp_ready = 1;
}
