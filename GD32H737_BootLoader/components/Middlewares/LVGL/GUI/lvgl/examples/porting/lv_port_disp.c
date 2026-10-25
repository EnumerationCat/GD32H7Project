/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#include "lcd.h"
#include <stdint.h>


#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "../../lvgl.h"


/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES (320) 
#define MY_DISP_VER_RES (480)

#define LV_VER_RES_MAX 480
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/



static volatile int lcd_bus_busy = 0;
static lv_disp_drv_t disp_drv; // 全局变量，供DMA中断回调使用
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/




void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

	
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* Example for 1) */
    // static lv_disp_draw_buf_t draw_buf_dsc_1;
    // static lv_color_t buf_1[MY_DISP_HOR_RES * 480];                          /*A buffer for 10 rows*/
    // lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 480);   /*Initialize the display buffer*/

    // /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 160];                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 160];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 160);   /*Initialize the display buffer*/
    
    // /* Example for 3) also set disp_drv.full_refresh = 1 below*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    //static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res =  lcddev.width;
    disp_drv.ver_res =  lcddev.height;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
	
    disp_drv.draw_buf = &draw_buf_dsc_2;
	
    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/

	lcd_init();                             /* 初始化LCD */
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    // uint16_t x, y;
    // uint16_t width = area->x2 - area->x1 + 1;
    // uint16_t height = area->y2 - area->y1 + 1;

    // /* Set the drawing window */
    // LCD_Set_Address_Window(area->x1, area->y1, width, height);

    // /* Write pixel data */
    // for(y = 0; y < height; y++) {
    //     for(x = 0; x < width; x++) {
    //         LCD_WR_DATA(color_p->full);
    //         color_p++;
    //     }
    // }



	// if(lcd_bus_busy) return;
	// lcd_bus_busy = 1; // 标记总线忙
	
	
    

	lcd_color_fill(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p);
	//LCD_Fill(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p);

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
	
    lv_disp_flush_ready(disp_drv);
	
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
