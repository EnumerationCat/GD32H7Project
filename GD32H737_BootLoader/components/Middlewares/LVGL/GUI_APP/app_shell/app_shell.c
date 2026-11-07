#include "app_shell.h"
#include "app_shell_cmd.h"
#include "lv_btn.h"
#include "lv_disp.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define APP_SHELL_LOG_SIZE   1024
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
static volatile int s_follow_bottom = 1;   /* 有本地输入时滚回最新位置 */

/* 输入行、历史命令、方向键状态：与 bootloader 的 finsh_shell 一致 */
static struct app_shell s_shell =
{
    .stat = WAIT_NORMAL,
    .echo_mode = 1,
    .prompt_mode = 1,
};

/* 回车后命令行等待刷新定时器执行 */
static volatile int s_cmd_ready = 0;

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

/* 调用者需处于临界区：把日志尾部 old_len 个字符替换成当前输入行 */
static void app_shell_log_replace_line(uint16_t old_len)
{
    uint16_t i;

    if(s_log_len >= old_len) {
        s_log_len -= old_len;
    } else {
        s_log_len = 0U;
    }
    for(i = 0U; i < s_shell.line_position; i++) {
        app_shell_log_putc(s_shell.line[i]);
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
    char line[APP_SHELL_CMD_SIZE + 1];
    uint16_t len;

    if(0 == s_cmd_ready) {
        return;
    }

    taskENTER_CRITICAL();
    len = s_shell.line_position;
    if(len > APP_SHELL_CMD_SIZE) {
        len = APP_SHELL_CMD_SIZE;
    }
    memcpy(line, s_shell.line, len);
    line[len] = '\0';
    s_shell.line_position = 0U;
    s_shell.line_curpos = 0U;
    s_shell.line[0] = '\0';
    s_cmd_ready = 0;
    taskEXIT_CRITICAL();

    app_shell_exec(line);
    if(s_shell.prompt_mode) {
        app_shell_putstr(APP_SHELL_PROMPT);
    }
}

/* ---------------- 历史命令 / 输入行重画 ---------------- */

static void app_shell_history_push(struct app_shell *sh)
{
    if(0U == sh->line_position) {
        sh->current_history = sh->history_count;
        return;
    }
    /* 与上一条相同则不重复记录 */
    if((sh->history_count > 0U) &&
       (0 == strcmp(sh->cmd_history[sh->history_count - 1U], sh->line))) {
        sh->current_history = sh->history_count;
        return;
    }

    if(sh->history_count >= APP_SHELL_HISTORY_LINES) {
        memmove(sh->cmd_history[0], sh->cmd_history[1],
                sizeof(sh->cmd_history[0]) * (APP_SHELL_HISTORY_LINES - 1U));
        sh->history_count = APP_SHELL_HISTORY_LINES - 1U;
    }

    memcpy(sh->cmd_history[sh->history_count], sh->line, sh->line_position + 1U);
    sh->history_count++;
    sh->current_history = sh->history_count;
}

/* 终端整行重画：擦掉旧提示符+输入行，重新输出，光标移回 line_curpos */
static void app_shell_term_redraw(uint16_t old_len)
{
    char erase[APP_SHELL_CMD_SIZE + 32];
    uint16_t back = (uint16_t)(s_shell.line_position - s_shell.line_curpos);
    uint32_t n = (uint32_t)(sizeof(APP_SHELL_PROMPT) - 1U) + old_len;

    if(n > (sizeof(erase) - 1U)) {
        n = sizeof(erase) - 1U;
    }
    memset(erase, ' ', n);
    erase[n] = '\0';

    printf("\r%s\r%s%s", erase, APP_SHELL_PROMPT, s_shell.line);
    while(back > 0U) {
        printf("\b");
        back--;
    }
}

/* 用历史命令替换当前输入行：LCD 日志尾部 + 终端整行重画 */
static void app_shell_history_recall(struct app_shell *sh, uint16_t index)
{
    uint16_t old_len;

    taskENTER_CRITICAL();

    /* 命令正在等待执行时不允许改输入行 */
    if(0 != s_cmd_ready) {
        taskEXIT_CRITICAL();
        return;
    }

    old_len = sh->line_position;

    if(index < sh->history_count) {
        memcpy(sh->line, sh->cmd_history[index], APP_SHELL_CMD_SIZE + 1U);
    } else {
        /* 翻过最后一条就是正在编辑的新行 */
        sh->line[0] = '\0';
    }
    sh->line_position = (uint16_t)strlen(sh->line);
    sh->line_curpos = sh->line_position;

    /* 当前输入的命令字符是日志的最后 old_len 个字符，先删掉再写入新命令 */
    app_shell_log_replace_line(old_len);
    s_log[s_log_len] = '\0';
    s_log_version++;
    s_follow_bottom = 1;

    taskEXIT_CRITICAL();

    /* 光标在行尾，重画后不需要回移 */
    if(sh->echo_mode) {
        app_shell_term_redraw(old_len);
    }
}

static void app_shell_history_up(struct app_shell *sh)
{
    if(sh->current_history > 0U) {
        sh->current_history--;
        app_shell_history_recall(sh, sh->current_history);
    }
}

static void app_shell_history_down(struct app_shell *sh)
{
    if(sh->current_history < sh->history_count) {
        sh->current_history++;
        app_shell_history_recall(sh, sh->current_history);
    }
}

#define APP_SHELL_ECHO_NONE       0
#define APP_SHELL_ECHO_CHAR       1
#define APP_SHELL_ECHO_BACKSPACE  2
#define APP_SHELL_ECHO_NEWLINE    3
#define APP_SHELL_ECHO_LEFT       4
#define APP_SHELL_ECHO_RIGHT      5
#define APP_SHELL_ECHO_REDRAW     6

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
        uint8_t c = data[i];
        char echo_ch = 0;
        uint16_t redraw_old = 0U;
        int echo = APP_SHELL_ECHO_NONE;

        /* 方向键：ESC [ A/B/C/D（部分终端是 ESC O A/B/C/D） */
        if(0x1B == c) {
            s_shell.stat = WAIT_SPEC_KEY;
            continue;
        }
        if(WAIT_SPEC_KEY == s_shell.stat) {
            if((0x5B == c) || (0x4F == c)) {
                s_shell.stat = WAIT_FUNC_KEY;
                continue;
            }
            s_shell.stat = WAIT_NORMAL;    /* 不是方向键序列，按普通键处理 */
        } else if(WAIT_FUNC_KEY == s_shell.stat) {
            s_shell.stat = WAIT_NORMAL;

            if(0x41 == c) {                 /* 上键：上一条历史 */
                app_shell_history_up(&s_shell);
            } else if(0x42 == c) {          /* 下键：下一条历史 */
                app_shell_history_down(&s_shell);
            } else if(0x44 == c) {          /* 左键：光标左移 */
                if(s_shell.line_curpos > 0U) {
                    s_shell.line_curpos--;
                    echo = APP_SHELL_ECHO_LEFT;
                }
            } else if(0x43 == c) {          /* 右键：光标右移 */
                if(s_shell.line_curpos < s_shell.line_position) {
                    echo_ch = s_shell.line[s_shell.line_curpos];
                    s_shell.line_curpos++;
                    echo = APP_SHELL_ECHO_RIGHT;
                }
            }

            if(s_shell.echo_mode) {
                if(APP_SHELL_ECHO_LEFT == echo) {
                    printf("\b");
                } else if(APP_SHELL_ECHO_RIGHT == echo) {
                    printf("%c", echo_ch);
                }
            }
            continue;
        }

        /* LF 跟在上一次 CR 后面时忽略，终端回车是 \r\n */
        if('\r' == c) {
            last_was_cr = 1;
        } else if('\n' == c) {
            if(last_was_cr) {
                last_was_cr = 0;
                continue;
            }
        } else {
            last_was_cr = 0;
        }

        taskENTER_CRITICAL();

        if(('\r' == c) || ('\n' == c)) {
            if(0 == s_cmd_ready) {
                s_shell.line[s_shell.line_position] = '\0';
                app_shell_log_putc('\n');
                app_shell_history_push(&s_shell);
                s_cmd_ready = 1;
                echo = APP_SHELL_ECHO_NEWLINE;
            }
        } else if(('\b' == c) || (0x7F == c)) {
            /* 退格：只删正在输入的命令字符；没有可删的不回显，
               终端光标就不会越过提示符继续左移 */
            if((0 == s_cmd_ready) && (s_shell.line_curpos > 0U)) {
                if(s_shell.line_curpos < s_shell.line_position) {
                    /* 行中间删除：LCD 日志里的整行重画 */
                    redraw_old = s_shell.line_position;
                    memmove(&s_shell.line[s_shell.line_curpos - 1U],
                            &s_shell.line[s_shell.line_curpos],
                            (uint32_t)(s_shell.line_position - s_shell.line_curpos) + 1U);
                    s_shell.line_position--;
                    s_shell.line_curpos--;
                    app_shell_log_replace_line(redraw_old);
                    echo = APP_SHELL_ECHO_REDRAW;
                } else {
                    s_shell.line_position--;
                    s_shell.line_curpos--;
                    s_shell.line[s_shell.line_position] = '\0';
                    if(s_log_len > 0U) {
                        s_log_len--;
                    }
                    echo = APP_SHELL_ECHO_BACKSPACE;
                }
            }
        } else if((c >= 0x20) && (c < 0x7F)) {
            if((0 == s_cmd_ready) && (s_shell.line_position < APP_SHELL_CMD_SIZE)) {
                if(s_shell.line_curpos < s_shell.line_position) {
                    /* 行中间插入：LCD 日志里的整行重画 */
                    redraw_old = s_shell.line_position;
                    memmove(&s_shell.line[s_shell.line_curpos + 1U],
                            &s_shell.line[s_shell.line_curpos],
                            (uint32_t)(s_shell.line_position - s_shell.line_curpos) + 1U);
                    s_shell.line[s_shell.line_curpos] = (char)c;
                    s_shell.line_position++;
                    s_shell.line_curpos++;
                    app_shell_log_replace_line(redraw_old);
                    echo = APP_SHELL_ECHO_REDRAW;
                } else {
                    s_shell.line[s_shell.line_position++] = (char)c;
                    s_shell.line_curpos++;
                    s_shell.line[s_shell.line_position] = '\0';
                    app_shell_log_putc((char)c);
                    echo_ch = (char)c;
                    echo = APP_SHELL_ECHO_CHAR;
                }
            }
        }

        if(APP_SHELL_ECHO_NONE != echo) {
            s_follow_bottom = 1;
        }

        s_log[s_log_len] = '\0';
        s_log_version++;
        taskEXIT_CRITICAL();

        /* 临界区外回显：退格必须发 "\b \b" 终端才会擦除字符 */
        if(0 == s_shell.echo_mode) {
            continue;
        }
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
        case APP_SHELL_ECHO_LEFT:
            printf("\b");
            break;
        case APP_SHELL_ECHO_RIGHT:
            printf("%c", echo_ch);
            break;
        case APP_SHELL_ECHO_REDRAW:
            app_shell_term_redraw(redraw_old);
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
    int follow;

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
    /* 本地输入（回显/回车/历史命令）时强制跟随最新，屏幕输出则只在已处
       底部时跟随，否则会把正在往回翻看的用户拽回底部 */
    follow = s_follow_bottom;
    s_follow_bottom = 0;
    taskEXIT_CRITICAL();

    if(0 == changed) {
        return;
    }

    if(0 == follow) {
        follow = (lv_obj_get_scroll_bottom(log_cont) <= 2);
    }

    /* 静态文本：set_text 每次都会 free + malloc 一整段文本，堆紧张时
       LV_ASSERT_MALLOC 会死循环（表现为偶尔卡死），静态文本不复制不分配 */
    lv_label_set_text_static(log_label, s_tmp);
    lv_obj_update_layout(log_cont);

    if(follow) {
        /* scroll_bottom 是“还能向下滚多少”，要加到当前 scroll_y 上才是绝对目标位置；
           直接把它当绝对位置会在顶部/底部之间来回跳，表现为屏幕上下来回抖 */
        lv_obj_scroll_to_y(log_cont,
                           lv_obj_get_scroll_y(log_cont) + lv_obj_get_scroll_bottom(log_cont),
                           LV_ANIM_OFF);
    }
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
    /* 去掉惯性/回弹：手指离开后不再继续滚动和重绘，滑动时 CPU 占用明显降低 */
    lv_obj_clear_flag(log_cont, LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ELASTIC);

    log_label = lv_label_create(log_cont);
    lv_obj_set_width(log_label, lv_pct(100));
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(log_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text_static(log_label, "");

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
