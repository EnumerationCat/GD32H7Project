#include "app_shell.h"
#include "lv_btn.h"
#include "lv_disp.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define APP_SHELL_LOG_SIZE   1024

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

/* 串口接收任务调用：把收到的字节追加到日志（\r\n 归一成单个换行） */
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

        /* 满了丢掉最早的一半，保留最近的输出 */
        if(s_log_len >= APP_SHELL_LOG_SIZE) {
            uint32_t keep = APP_SHELL_LOG_SIZE / 2U;
            memmove(s_log, s_log + (s_log_len - keep), keep);
            s_log_len = keep;
        }
        s_log[s_log_len++] = c;
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
