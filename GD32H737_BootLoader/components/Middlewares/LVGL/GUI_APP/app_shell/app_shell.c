#include "app_shell.h"
#include "lv_btn.h"
#include "lv_disp.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "gd32h7xx.h"

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

/* 以下日志操作要求调用者已进入临界区 */
static void app_shell_log_putc(char c)
{
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
static void app_shell_out(const char *s)
{
    taskENTER_CRITICAL();
    app_shell_log_puts(s);
    s_log[s_log_len] = '\0';
    s_log_version++;
    taskEXIT_CRITICAL();

    printf("%s", s);
}

static int app_shell_streq(const char *line, const char *cmd)
{
    uint32_t n = (uint32_t)strlen(cmd);

    if(0 != strncmp(line, cmd, n)) {
        return 0;
    }

    /* 命令名之后只允许空白 */
    while((' ' == line[n]) || ('\t' == line[n])) {
        n++;
    }

    return ('\0' == line[n]);
}

static void app_shell_exec(const char *line)
{
    while((' ' == *line) || ('\t' == *line)) {
        line++;
    }

    if('\0' == *line) {
        /* 空行：什么都不做，只补新提示符 */
    } else if(app_shell_streq(line, "clear")) {
        taskENTER_CRITICAL();
        s_log_len = 0;
        app_shell_log_puts(APP_SHELL_PROMPT);
        s_log[s_log_len] = '\0';
        s_log_version++;
        taskEXIT_CRITICAL();

        printf("\r\n%s", APP_SHELL_PROMPT);
        return;
    } else if(app_shell_streq(line, "reboot")) {
        app_shell_out("[reboot] system reset\r\n");
        NVIC_SystemReset();
        return;
    } else {
        app_shell_out("unknown command: ");
        app_shell_out(line);
        app_shell_out("\r\n");
    }

    app_shell_out(APP_SHELL_PROMPT);
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
}

/* 串口接收任务调用：把收到的字节追加到日志（\r\n 归一成单个换行），
   并缓存一行命令，回车后由刷新定时器执行 */
void app_shell_rx_push(const uint8_t *data, uint32_t size)
{
    static uint8_t last_was_cr = 0;
    uint32_t i;

    if((NULL == data) || (0U == size)) {
        return;
    }

    taskENTER_CRITICAL();
    for(i = 0U; i < size; i++) {
        char c = (char)data[i];

        if('\r' == c) {
            c = '\n';
            last_was_cr = 1;
        } else if('\n' == c) {
            if(last_was_cr) {
                last_was_cr = 0;
                continue;
            }
        } else {
            last_was_cr = 0;
        }

        if('\n' == c) {
            if(0 == s_cmd_ready) {
                app_shell_log_putc('\n');
                s_cmd[s_cmd_len] = '\0';
                s_cmd_ready = 1;
            }
        } else if(('\b' == c) || (0x7F == c)) {
            /* 退格：只删正在输入的命令字符 */
            if((0 == s_cmd_ready) && (s_cmd_len > 0U)) {
                s_cmd_len--;
                if(s_log_len > 0U) {
                    s_log_len--;
                }
            }
        } else if((c >= 0x20) && (c < 0x7F)) {
            if((0 == s_cmd_ready) && (s_cmd_len < APP_SHELL_CMD_MAX)) {
                s_cmd[s_cmd_len++] = c;
                app_shell_log_putc(c);
            }
        }
    }
    s_log[s_log_len] = '\0';
    s_log_version++;
    taskEXIT_CRITICAL();
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
    lv_obj_scroll_to_y(log_cont, lv_obj_get_scroll_bottom(log_cont), LV_ANIM_OFF);
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

    /* 右上角 BACK */
    lv_obj_t *back = lv_btn_create(scr);
    lv_obj_set_size(back, 70, 40);
    lv_obj_align(back, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(back, app_shell_back_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back);
    lv_label_set_text(back_label, "BACK");
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
