#include "app_shell.h"
#include "app_shell_cmd.h"
#include "lv_btn.h"
#include "lv_disp.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define APP_SHELL_LOG_SIZE   1024
#define APP_SHELL_CMD_MAX    64
#define APP_SHELL_PROMPT     "app_shell> "

static lv_obj_t *home_screen = NULL;
static lv_obj_t *shell_screen = NULL;
static lv_obj_t *log_cont = NULL;
static lv_obj_t *log_label = NULL;
static lv_timer_t *refresh_timer = NULL;

static char s_log[APP_SHELL_LOG_SIZE + 1];
static uint32_t s_log_len = 0;
static volatile uint32_t s_log_version = 0;
static uint32_t s_shown_version = 0;
static char s_tmp[APP_SHELL_LOG_SIZE + 1];

/* 当前正在输入的一行命令，回车后由刷新定时器执行 */
static char s_cmd[APP_SHELL_CMD_MAX + 1];
static uint32_t s_cmd_len = 0;
static volatile int s_cmd_ready = 0;

/* 历史命令：上下键翻看，行为与 bootloader shell 一致 */
#define APP_SHELL_HISTORY_LINES  10
static char s_history[APP_SHELL_HISTORY_LINES][APP_SHELL_CMD_MAX + 1];
static int s_history_count = 0;
static int s_history_cur = 0;     /* 0..count，count 表示正在编辑的新行 */
static int s_esc_state = 0;       /* 0 无，1 收到 ESC，2 收到 ESC [，3 收到 ESC O */

/* 以下日志操作要求调用者已进入临界区 */
static void app_shell_log_putc(char c)
{
    /* LCD 标签只按 '\n' 换行：输出里的 "\r\n" 若把 '\r' 也存进日志，
       会被当成额外一次换行，表现为多出一行空行 */
    if('\r' == c) {
        return;
    }

    /* 满了丢掉最早的一半，保留最近的输出 */
    if(s_log_len >= APP_SHELL_LOG_SIZE) {
        uint32_t keep = APP_SHELL_LOG_SIZE / 2U;
        memmove(s_log, s_log + (s_log_len - keep), keep);
        s_log_len = keep;
    }
    s_log[s_log_len++] = c;
}

static void app_shell_log_puts(const char *s)
{
    while('\0' != *s) {
        app_shell_log_putc(*s++);
    }
}

/* 同时输出到串口和页面日志 */
void app_shell_putstr(const char *s)
{
    if(NULL == s) {
        return;
    }

    taskENTER_CRITICAL();
    app_shell_log_puts(s);
    s_log[s_log_len] = '\0';
    s_log_version++;
    taskEXIT_CRITICAL();

    printf("%s", s);
}

void app_shell_clear(void)
{
    taskENTER_CRITICAL();
    s_log_len = 0;
    s_log[0] = '\0';
    s_log_version++;
    taskEXIT_CRITICAL();
}

/* 执行回车留下的一整行命令（在 LVGL 定时器里调用，可安全使用 printf） */
static void app_shell_run_pending(void)
{
    char line[APP_SHELL_CMD_MAX + 1];

    if(0 == s_cmd_ready) {
        return;
    }

    taskENTER_CRITICAL();
    memcpy(line, s_cmd, s_cmd_len);
    line[s_cmd_len] = '\0';
    s_cmd_len = 0;
    s_cmd_ready = 0;
    taskEXIT_CRITICAL();

    app_shell_exec(line);
    app_shell_putstr(APP_SHELL_PROMPT);
}

/* ---------------- 历史命令（上下键） ---------------- */

static void app_shell_history_push(const char *line)
{
    size_t n;

    if('\0' == line[0]) {
        s_history_cur = s_history_count;
        return;
    }
    /* 与上一条相同则不重复记录 */
    if((s_history_count > 0) &&
       (0 == strcmp(s_history[s_history_count - 1], line))) {
        s_history_cur = s_history_count;
        return;
    }

    if(s_history_count >= APP_SHELL_HISTORY_LINES) {
        memmove(s_history[0], s_history[1],
                sizeof(s_history[0]) * (APP_SHELL_HISTORY_LINES - 1));
        s_history_count = APP_SHELL_HISTORY_LINES - 1;
    }

    n = strlen(line);
    if(n > APP_SHELL_CMD_MAX) {
        n = APP_SHELL_CMD_MAX;
    }
    memcpy(s_history[s_history_count], line, n);
    s_history[s_history_count][n] = '\0';
    s_history_count++;
    s_history_cur = s_history_count;
}

/* 用历史命令替换当前输入行：LCD 日志尾部 + 终端整行重画 */
static void app_shell_history_recall(int index)
{
    char line[APP_SHELL_CMD_MAX + 1];
    char erase[APP_SHELL_CMD_MAX + 32];
    uint32_t old_len;
    uint32_t new_len;
    uint32_t n;

    taskENTER_CRITICAL();

    /* 命令正在等待执行时不允许改输入行 */
    if(0 != s_cmd_ready) {
        taskEXIT_CRITICAL();
        return;
    }

    if(index < s_history_count) {
        memcpy(line, s_history[index], sizeof(line));
        line[APP_SHELL_CMD_MAX] = '\0';
    } else {
        line[0] = '\0';
    }

    old_len = s_cmd_len;
    new_len = (uint32_t)strlen(line);

    /* 当前输入的命令字符是日志的最后 old_len 个字符，先删掉再写入新命令 */
    if(s_log_len >= old_len) {
        s_log_len -= old_len;
    } else {
        s_log_len = 0U;
    }
    memcpy(s_cmd, line, new_len + 1U);
    s_cmd_len = new_len;
    for(uint32_t i = 0U; i < new_len; i++) {
        app_shell_log_putc(line[i]);
    }
    s_log[s_log_len] = '\0';
    s_log_version++;

    taskEXIT_CRITICAL();

    /* "\r" + 空格擦掉旧行 + "\r" + 提示符 + 新命令 */
    n = (uint32_t)(sizeof(APP_SHELL_PROMPT) - 1U) + old_len;
    if(n > (sizeof(erase) - 1U)) {
        n = sizeof(erase) - 1U;
    }
    memset(erase, ' ', n);
    erase[n] = '\0';

    printf("\r%s\r%s%s", erase, APP_SHELL_PROMPT, line);
}

static void app_shell_history_up(void)
{
    if(s_history_cur > 0) {
        s_history_cur--;
        app_shell_history_recall(s_history_cur);
    }
}

static void app_shell_history_down(void)
{
    if(s_history_cur < s_history_count) {
        s_history_cur++;
        app_shell_history_recall(s_history_cur);
    }
}

#define APP_SHELL_ECHO_NONE       0
#define APP_SHELL_ECHO_CHAR       1
#define APP_SHELL_ECHO_BACKSPACE  2
#define APP_SHELL_ECHO_NEWLINE    3

/* 串口接收任务调用：把收到的字节追加到日志（\r\n 归一成单个换行），
   缓存一行命令，回车后由刷新定时器执行；同时负责回显到终端 */
void app_shell_rx_push(const uint8_t *data, uint32_t size)
{
    static uint8_t last_was_cr = 0;
    uint32_t i;

    if((NULL == data) || (0U == size)) {
        return;
    }

    for(i = 0U; i < size; i++) {
        char c = (char)data[i];
        char echo_ch = 0;
        int echo = APP_SHELL_ECHO_NONE;

        /* 方向键：ESC [ A/B（部分终端是 ESC O A/B）上翻/下翻历史 */
        if(0x1B == data[i]) {
            s_esc_state = 1;
            continue;
        }
        if(1 == s_esc_state) {
            if(0x5B == data[i]) {
                s_esc_state = 2;        /* '[' */
            } else if(0x4F == data[i]) {
                s_esc_state = 3;        /* 'O' */
            } else {
                s_esc_state = 0;
            }
            continue;
        }
        if((2 == s_esc_state) || (3 == s_esc_state)) {
            s_esc_state = 0;
            if(0x41 == data[i]) {
                app_shell_history_up();
            } else if(0x42 == data[i]) {
                app_shell_history_down();
            }
            continue;
        }

        taskENTER_CRITICAL();

        if('\r' == c) {
            c = '\n';
            last_was_cr = 1;
        } else if('\n' == c) {
            if(last_was_cr) {
                last_was_cr = 0;
                taskEXIT_CRITICAL();
                continue;
            }
        } else {
            last_was_cr = 0;
        }

        if('\n' == c) {
            if(0 == s_cmd_ready) {
                app_shell_log_putc('\n');
                s_cmd[s_cmd_len] = '\0';
                app_shell_history_push(s_cmd);
                s_cmd_ready = 1;
                echo = APP_SHELL_ECHO_NEWLINE;
            }
        } else if(('\b' == c) || (0x7F == c)) {
            /* 退格：只删正在输入的命令字符；没有可删的不回显，
               终端光标就不会越过提示符继续左移 */
            if((0 == s_cmd_ready) && (s_cmd_len > 0U)) {
                s_cmd_len--;
                if(s_log_len > 0U) {
                    s_log_len--;
                }
                echo = APP_SHELL_ECHO_BACKSPACE;
            }
        } else if((c >= 0x20) && (c < 0x7F)) {
            if((0 == s_cmd_ready) && (s_cmd_len < APP_SHELL_CMD_MAX)) {
                s_cmd[s_cmd_len++] = c;
                app_shell_log_putc(c);
                echo_ch = c;
                echo = APP_SHELL_ECHO_CHAR;
            }
        }

        s_log[s_log_len] = '\0';
        s_log_version++;
        taskEXIT_CRITICAL();

        /* 临界区外回显：退格必须发 "\b \b" 终端才会擦除字符 */
        switch(echo) {
        case APP_SHELL_ECHO_CHAR:
            printf("%c", echo_ch);
            break;
        case APP_SHELL_ECHO_BACKSPACE:
            printf("\b \b");
            break;
        case APP_SHELL_ECHO_NEWLINE:
            printf("\r\n");
            break;
        default:
            break;
        }
    }
}

static void app_shell_refresh_cb(lv_timer_t *timer)
{
    uint32_t len;
    int changed = 0;

    (void)timer;

    app_shell_run_pending();

    if(NULL == log_label) {
        return;
    }

    taskENTER_CRITICAL();
    if(s_log_version != s_shown_version) {
        len = s_log_len;
        if(len > APP_SHELL_LOG_SIZE) {
            len = APP_SHELL_LOG_SIZE;
        }
        memcpy(s_tmp, s_log, len);
        s_tmp[len] = '\0';
        s_shown_version = s_log_version;
        changed = 1;
    }
    taskEXIT_CRITICAL();

    if(0 == changed) {
        return;
    }

    lv_label_set_text(log_label, s_tmp);
    lv_obj_update_layout(log_cont);

    /* scroll_bottom 是“还能向下滚多少”，要加到当前 scroll_y 上才是绝对目标位置；
       直接把它当绝对位置会在顶部/底部之间来回跳，表现为屏幕上下来回抖 */
    lv_obj_scroll_to_y(log_cont,
                       lv_obj_get_scroll_y(log_cont) + lv_obj_get_scroll_bottom(log_cont),
                       LV_ANIM_OFF);
}

static void app_shell_back_cb(lv_event_t *e)
{
    if((LV_EVENT_CLICKED == lv_event_get_code(e)) && (NULL != home_screen)) {
        lv_scr_load(home_screen);
    }
}

static void app_shell_build(lv_obj_t *scr)
{
    /* 背景全黑 */
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    /* 右上角关闭按钮 */
    lv_obj_t *back = lv_btn_create(scr);
    lv_obj_set_size(back, 40, 40);
    lv_obj_set_style_radius(back, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(back, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(back, app_shell_back_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back);
    lv_label_set_text(back_label, "X");
    lv_obj_center(back_label);

    /* 串口日志显示区，可上下滚动 */
    log_cont = lv_obj_create(scr);
    lv_obj_set_size(log_cont, LV_HOR_RES - 20, LV_VER_RES - 70);
    lv_obj_align(log_cont, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(log_cont, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(log_cont, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(log_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(log_cont, 6, LV_PART_MAIN);
    lv_obj_set_scroll_dir(log_cont, LV_DIR_VER);

    log_label = lv_label_create(log_cont);
    lv_obj_set_width(log_label, lv_pct(100));
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(log_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(log_label, "");

    /* 首次进入时给出命令行提示符 */
    {
        int need_nl;

        taskENTER_CRITICAL();
        need_nl = ((s_log_len > 0U) && ('\n' != s_log[s_log_len - 1U]));
        if(need_nl) {
            app_shell_log_putc('\n');
        }
        app_shell_log_puts(APP_SHELL_PROMPT);
        s_log[s_log_len] = '\0';
        s_log_version++;
        taskEXIT_CRITICAL();

        printf(need_nl ? "\r\n" : "");
        printf(APP_SHELL_PROMPT);
    }

    refresh_timer = lv_timer_create(app_shell_refresh_cb, 200, NULL);
}

void app_shell(void)
{
    /* 从 user_home 进入时记录当前屏幕，用于返回 */
    home_screen = lv_scr_act();

    if(NULL == shell_screen) {
        shell_screen = lv_obj_create(NULL);
        app_shell_build(shell_screen);
    }

    lv_scr_load(shell_screen);
}
