/**
 ****************************************************************************************************
 * @file        lcd.h
 * @version     V1.0
 * @brief       3.5寸 TFTLCD(MCU屏) 驱动代码	
 *              支持驱动IC型号:ST7796
 ****************************************************************************************************
 *
 * 实验平台:    GD32H737VMT6小系统板
 *
 ****************************************************************************************************
 */
#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "sys.h"


/******************************************************************************************/
/* LCD RST/BL/WR/RD/CS/RS 引脚 定义 
 * LCD_D0~D15,由于引脚太多,就不在这里定义了,直接在lcd_init里面修改.所以在移植的时候,除了改
 * 这6个IO口, 还得改lcd_init里面的D0~D15所在的IO口.
 */

#define LCD_RST_GPIO_PORT               GPIOA
#define LCD_RST_GPIO_PIN                GPIO_PIN_8
#define LCD_RST_GPIO_CLK                RCU_GPIOA     /* GPIOA时钟使能 */

#define LCD_BL_GPIO_PORT                GPIOB
#define LCD_BL_GPIO_PIN                 GPIO_PIN_1
#define LCD_BL_GPIO_CLK                 RCU_GPIOB     /* GPIOB时钟使能 */

#define LCD_WR_GPIO_PORT                GPIOD
#define LCD_WR_GPIO_PIN                 GPIO_PIN_5
#define LCD_WR_GPIO_CLK                 RCU_GPIOD     /* GPIOD时钟使能 */

#define LCD_RD_GPIO_PORT                GPIOD
#define LCD_RD_GPIO_PIN                 GPIO_PIN_4
#define LCD_RD_GPIO_CLK                 RCU_GPIOD     /* GPIOD时钟使能 */
 
/* LCD_CS(需要根据LCD_EXMC_NEX设置正确的IO口) 和 LCD_RS(需要根据LCD_EXMC_AX设置正确的IO口) 引脚 定义 */
#define LCD_CS_GPIO_PORT                GPIOD
#define LCD_CS_GPIO_PIN                 GPIO_PIN_7
#define LCD_CS_GPIO_CLK                 RCU_GPIOD     /* GPIOD时钟使能 */

#define LCD_RS_GPIO_PORT                GPIOE
#define LCD_RS_GPIO_PIN                 GPIO_PIN_3
#define LCD_RS_GPIO_CLK                 RCU_GPIOE     /* GPIOE时钟使能 */

/* EXMC相关参数 定义 
 * 注意: 我们默认是通过EXMC块0来连接LCD, 块0有4个片选: EXMC_NE0~3
 *
 * 修改LCD_EXMC_NEX, 对应的LCD_CS_GPIO相关设置也得改
 * 修改LCD_EXMC_AX , 对应的LCD_RS_GPIO相关设置也得改
 */ 
#define LCD_EXMC_NEX             0              /* 使用EXMC_NE0接LCD_CS,取值范围只能是: 0 ~ 3 */
#define LCD_EXMC_AX              19             /* 使用EXMC_A19接LCD_RS,取值范围是: 0 ~ 25 */

#define LCD_EXMC_SNWTCFGX        EXMC_SNWTCFG(LCD_EXMC_NEX)     /* 写时序寄存器(EXMC_SNWTCFGx,x=0,1,2,3),根据LCD_EXMC_NEX自动确定 */

/******************************************************************************************/

/* LCD重要参数集 */
typedef struct
{
    uint16_t width;     /* LCD 宽度 */
    uint16_t height;    /* LCD 高度 */
    uint16_t id;        /* LCD ID */
    uint8_t dir;        /* 横屏还是竖屏控制：0，竖屏；1，横屏。 */
    uint16_t wramcmd;   /* 开始写gram指令 */
    uint16_t setxcmd;   /* 设置x坐标指令 */
    uint16_t setycmd;   /* 设置y坐标指令 */
} _lcd_dev;

/* LCD参数 */
extern _lcd_dev lcddev; /* 管理LCD重要参数 */

/* LCD的画笔颜色和背景色 */
extern uint32_t  g_point_color;     /* 默认红色 */
extern uint32_t  g_back_color;      /* 背景颜色.默认为白色 */

/* LCD背光控制 */
#define LCD_BL(x)        do{ x ? \
                             gpio_bit_write(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, SET) : \
                             gpio_bit_write(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, RESET); \
                         }while(0)     

/* LCD复位引脚 */
#define LCD_RST(x)       do{ x ? \
                             gpio_bit_write(LCD_RST_GPIO_PORT, LCD_RST_GPIO_PIN, SET) : \
                             gpio_bit_write(LCD_RST_GPIO_PORT, LCD_RST_GPIO_PIN, RESET); \
                         }while(0)     

/* LCD地址结构体 */
typedef struct
{
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
} LCD_TypeDef;


/* LCD_BASE的详细计算方法:
 * 我们一般使用EXMC的块0(BANK0)来驱动TFTLCD液晶屏(MCU屏), 块0地址范围总大小为256MB,均分成4块:
 * 存储块0(EXMC_NE0)地址范围: 0X6000 0000 ~ 0X63FF FFFF
 * 存储块1(EXMC_NE1)地址范围: 0X6400 0000 ~ 0X67FF FFFF
 * 存储块2(EXMC_NE2)地址范围: 0X6800 0000 ~ 0X6BFF FFFF
 * 存储块3(EXMC_NE3)地址范围: 0X6C00 0000 ~ 0X6FFF FFFF
 *
 * 我们需要根据硬件连接方式选择合适的片选(连接LCD_CS)和地址线(连接LCD_RS)
 * 开发板使用EXMC_NE0连接LCD_CS, EXMC_A19连接LCD_RS ,16位数据线,计算方法如下:
 * 首先EXMC_NE0的基地址为: 0X6000 0000;     NEx的基址为(x=0/1/2/3): 0X6000 0000 + (0X400 0000 * x)
 * EXMC_A19对应地址值: 2^19 * 2 = 0X100000;       EXMC_Ay对应的地址为(y = 0 ~ 25): 2^y * 2
 *
 * LCD->LCD_REG,对应LCD_RS = 0(LCD寄存器); LCD->LCD_RAM,对应LCD_RS = 1(LCD数据)
 * 则 LCD->LCD_RAM的地址为:  0X6000 0000 + 2^19 * 2 = 0X6010 0000
 *    LCD->LCD_REG的地址可以为 LCD->LCD_RAM之外的任意地址.
 * 由于我们使用结构体管理LCD_REG 和 LCD_RAM(REG在前,RAM在后,均为16位数据宽度)
 * 因此 结构体的基地址(LCD_BASE) = LCD_RAM - 2 = 0X6010 0000 - 2
 *
 * 更加通用的计算公式为((片选脚EXMC_NEx)x=0/1/2/3, (RS接地址线EXMC_Ay)y=0~25):
 *          LCD_BASE = (0X6000 0000 + (0X400 0000 * x)) | (2^y * 2 - 2)
 *          等效于(使用移位操作)
 *          LCD_BASE = (0X6000 0000 + (0X400 0000 * x)) | ((1 << y) * 2 - 2)
 */
#define LCD_BASE        (uint32_t)((0X60000000 + (0X4000000 * LCD_EXMC_NEX)) | (((1 << LCD_EXMC_AX) * 2) - 2))
#define LCD             ((LCD_TypeDef *) LCD_BASE)

/******************************************************************************************/
/* LCD扫描方向和颜色 定义 */

/* 扫描方向定义 */
#define L2R_U2D         0           /* 从左到右,从上到下 */
#define L2R_D2U         1           /* 从左到右,从下到上 */
#define R2L_U2D         2           /* 从右到左,从上到下 */
#define R2L_D2U         3           /* 从右到左,从下到上 */

#define U2D_L2R         4           /* 从上到下,从左到右 */
#define U2D_R2L         5           /* 从上到下,从右到左 */
#define D2U_L2R         6           /* 从下到上,从左到右 */
#define D2U_R2L         7           /* 从下到上,从右到左 */

#define DFT_SCAN_DIR    L2R_U2D     /* 默认的扫描方向 */

/* 常用画笔颜色 */
#define WHITE           0xFFFF      /* 白色 */
#define BLACK           0x0000      /* 黑色 */
#define RED             0xF800      /* 红色 */
#define GREEN           0x07E0      /* 绿色 */
#define BLUE            0x001F      /* 蓝色 */ 
#define MAGENTA         0XF81F      /* 品红色/紫红色 = BLUE + RED */
#define YELLOW          0XFFE0      /* 黄色 = GREEN + RED */
#define CYAN            0X07FF      /* 青色 = GREEN + BLUE */  

/* 非常用颜色 */
#define BROWN           0XBC40      /* 棕色 */
#define BRRED           0XFC07      /* 棕红色 */
#define GRAY            0X8430      /* 灰色 */ 
#define DARKBLUE        0X01CF      /* 深蓝色 */
#define LIGHTBLUE       0X7D7C      /* 浅蓝色 */ 
#define GRAYBLUE        0X5458      /* 灰蓝色 */ 
#define LIGHTGREEN      0X841F      /* 浅绿色 */  
#define LGRAY           0XC618      /* 浅灰色(PANNEL),窗体背景色 */ 
#define LGRAYBLUE       0XA651      /* 浅灰蓝色(中间层颜色) */ 
#define LBBLUE          0X2B12      /* 浅棕蓝色(选择条目的反色) */ 
   
/******************************************************************************************/
/* 函数声明 */
  
void lcd_wr_data(volatile uint16_t data);            /* LCD写数据 */
void lcd_wr_regno(volatile uint16_t regno);          /* LCD写寄存器编号/地址 */
void lcd_write_reg(uint16_t regno, uint16_t data);   /* LCD写寄存器的值 */

void lcd_init(void);                                 /* 初始化LCD */ 
void exmc_sram_init(void);                           /* 初始化EXMC */  
void lcd_display_on(void);                           /* 开显示 */ 
void lcd_display_off(void);                          /* 关显示 */
void lcd_scan_dir(uint8_t dir);                      /* 设置屏幕扫描方向 */ 
void lcd_display_dir(uint8_t dir);                   /* 设置屏幕显示方向 */ 

void lcd_write_ram_prepare(void);                              /* 准备写GRAM */ 
void lcd_set_cursor(uint16_t x, uint16_t y);                   /* 设置光标 */ 
uint32_t lcd_read_point(uint16_t x, uint16_t y);               /* 读点 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color);   /* 画点 */

void lcd_clear(uint16_t color);                                                                /* LCD清屏 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                      /* 填充实心圆 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);                     /* 画圆 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color);                     /* 画水平线 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);                /* 设置窗口 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);             /* 纯色填充矩形 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);      /* 彩色填充矩形 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);        /* 画直线 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);   /* 画矩形 */

void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color);                       /* 显示一个字符 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color);                     /* 显示数字 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color);      /* 扩展显示数字 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color);   /* 显示字符串 */



#ifdef __cplusplus
}
#endif

#endif
