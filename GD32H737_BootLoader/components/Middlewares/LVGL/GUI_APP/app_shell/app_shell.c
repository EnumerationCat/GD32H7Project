#include "app_shell.h"
#include "lv_btn.h"
#include "lv_disp.h"

static lv_obj_t *home_screen = NULL;
static lv_obj_t *shell_screen = NULL;

/* 返回 user_home 页面 */
static void app_shell_back_cb(lv_event_t *e)
{
    if((LV_EVENT_CLICKED == lv_event_get_code(e)) && (NULL != home_screen)) {
        lv_scr_load(home_screen);
    }
}

static void app_shell_build(lv_obj_t *scr)
{
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "APP SHELL");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *back = lv_btn_create(scr);
    lv_obj_set_size(back, 80, 50);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(back, app_shell_back_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);
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
