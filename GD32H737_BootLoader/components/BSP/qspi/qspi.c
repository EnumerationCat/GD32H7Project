/**
 ****************************************************************************************************
 * @file        qspi.c
 * @version     V1.0
 * @brief       QSPI 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H737VMT6小系统板
 *
 ****************************************************************************************************
 */	
 
#include "qspi.h"
#include "usart.h"


ospi_parameter_struct g_ospi_struct;   /* OSPI参数结构体 */

/**
 * @brief       初始化OSPI接口
 * @param       ospi_periph : OSPI接口，OSPIx(x=0,1)
 * @param       ospi_struct : OSPI参数结构体
 * @retval      无
 */
void ospi_flash_init(uint32_t ospi_periph, ospi_parameter_struct *ospi_struct)
{
    ospi_deinit(ospi_periph);                          /* 复位OSPI */
    ospim_deinit();                                    /* 复位OSPIM */
  
    rcu_periph_clock_enable(RCU_OSPIM);                /* 使能OSPIM时钟 */  
    rcu_periph_clock_enable(OSPIM_P0_SCK_GPIO_CLK);    /* 使能OSPIM_P0_SCK IO口时钟 */
    rcu_periph_clock_enable(OSPIM_P0_CSN_GPIO_CLK);    /* 使能OSPIM_P0_CSN IO口时钟 */
    rcu_periph_clock_enable(OSPIM_P0_IO0_GPIO_CLK);    /* 使能OSPIM_P0_IO0 IO口时钟 */
    rcu_periph_clock_enable(OSPIM_P0_IO1_GPIO_CLK);    /* 使能OSPIM_P0_IO1 IO口时钟 */
    rcu_periph_clock_enable(OSPIM_P0_IO2_GPIO_CLK);    /* 使能OSPIM_P0_IO2 IO口时钟 */
    rcu_periph_clock_enable(OSPIM_P0_IO3_GPIO_CLK);    /* 使能OSPIM_P0_IO3 IO口时钟 */
  
    /* 配置使用的OSPIM引脚:
           OSPIM_P0_SCK(PB10)
           OSPIM_P0_CSN(PB2)
           OSPIM_P0_IO0(PD11)
           OSPIM_P0_IO1(PD12)
           OSPIM_P0_IO2(PE2)
           OSPIM_P0_IO3(PD13) */

    /* 配置OSPIM引脚的复用功能 */
    gpio_af_set(OSPIM_P0_SCK_GPIO_PORT, OSPIM_P0_SCK_GPIO_AF, OSPIM_P0_SCK_GPIO_PIN);
    gpio_af_set(OSPIM_P0_CSN_GPIO_PORT, OSPIM_P0_CSN_GPIO_AF, OSPIM_P0_CSN_GPIO_PIN);  
    gpio_af_set(OSPIM_P0_IO0_GPIO_PORT, OSPIM_P0_IO0_GPIO_AF, OSPIM_P0_IO0_GPIO_PIN);
    gpio_af_set(OSPIM_P0_IO1_GPIO_PORT, OSPIM_P0_IO1_GPIO_AF, OSPIM_P0_IO1_GPIO_PIN);
    gpio_af_set(OSPIM_P0_IO2_GPIO_PORT, OSPIM_P0_IO2_GPIO_AF, OSPIM_P0_IO2_GPIO_PIN);
    gpio_af_set(OSPIM_P0_IO3_GPIO_PORT, OSPIM_P0_IO3_GPIO_AF, OSPIM_P0_IO3_GPIO_PIN);
    
    /* OSPIM_P0_SCK引脚模式设置 */
    gpio_mode_set(OSPIM_P0_SCK_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_SCK_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_SCK_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_SCK_GPIO_PIN);

    /* OSPIM_P0_CSN引脚模式设置 */
    gpio_mode_set(OSPIM_P0_CSN_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_CSN_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_CSN_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_CSN_GPIO_PIN);

    /* OSPIM_P0_IO0引脚模式设置 */
    gpio_mode_set(OSPIM_P0_IO0_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_IO0_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_IO0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_IO0_GPIO_PIN);

    /* OSPIM_P0_IO1引脚模式设置 */
    gpio_mode_set(OSPIM_P0_IO1_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_IO1_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_IO1_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_IO1_GPIO_PIN);

    /* OSPIM_P0_IO2引脚模式设置 */
    gpio_mode_set(OSPIM_P0_IO2_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_IO2_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_IO2_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_IO2_GPIO_PIN);

    /* OSPIM_P0_IO3引脚模式设置 */
    gpio_mode_set(OSPIM_P0_IO3_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, OSPIM_P0_IO3_GPIO_PIN);
    gpio_output_options_set(OSPIM_P0_IO3_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, OSPIM_P0_IO3_GPIO_PIN);
    
    ospim_port_sck_config(OSPIM_PORT0, OSPIM_PORT_SCK_ENABLE);    /* 使能OSPIM端口0的SCK */
    ospim_port_csn_config(OSPIM_PORT0, OSPIM_PORT_CSN_ENABLE);    /* 使能OSPIM端口0的CSN */
    ospim_port_io3_0_config(OSPIM_PORT0, OSPIM_IO_LOW_ENABLE);    /* 使能OSPIM端口0的IO[3:0] */

    switch (ospi_periph) 
    {
        case OSPI0:    
            rcu_periph_clock_enable(RCU_OSPI0);                                        /* 使能OSPI0时钟 */
            ospim_port_sck_source_select(OSPIM_PORT0, OSPIM_SCK_SOURCE_OSPI0_SCK);     /* 选择OSPIM端口0的SCK源为OSPI0_SCK */
            ospim_port_csn_source_select(OSPIM_PORT0, OSPIM_CSN_SOURCE_OSPI0_CSN);     /* 选择OSPIM端口0的CSN源为OSPI0_CSN */
            ospim_port_io3_0_source_select(OSPIM_PORT0, OSPIM_SRCPLIO_OSPI0_IO_LOW);   /* 选择OSPIM端口0的IO[3:0]源为OSPI0_IO[3:0] */
            break;
        
        case OSPI1:
            rcu_periph_clock_enable(RCU_OSPI1);                                        /* 使能OSPI1时钟 */
            ospim_port_sck_source_select(OSPIM_PORT0, OSPIM_SCK_SOURCE_OSPI1_SCK);     /* 选择OSPIM端口0的SCK源为OSPI1_SCK */
            ospim_port_csn_source_select(OSPIM_PORT0, OSPIM_CSN_SOURCE_OSPI1_CSN);     /* 选择OSPIM端口0的CSN源为OSPI1_CSN */
            ospim_port_io3_0_source_select(OSPIM_PORT0, OSPIM_SRCPLIO_OSPI1_IO_LOW);   /* 选择OSPIM端口0的IO[3:0]源为OSPI1_IO[3:0] */
            break;
        
        default:
            break;
    }

    ospi_struct_init(ospi_struct);                                 /* 初始化OSPI结构体参数为默认值 */

    ospi_struct->prescaler = (4 - 1);                              /* 从内核时钟分频产生OSPI时钟的分频因子 
                                                                    * OSPI时钟来自AHB3时钟，设置使用4分频, 则OSPI时钟为300M / 4 = 75Mhz      
                                                                    */
    ospi_struct->sample_shift = OSPI_SAMPLE_SHIFTING_HALF_CYCLE;   /* 采样移位二分之一个周期 */
    ospi_struct->fifo_threshold = OSPI_FIFO_THRESHOLD_4;           /* FIFO中的阈值为4个字节 */
    ospi_struct->device_size = OSPI_MESZ_16_MBS;                   /* 外部存储器大小为16MB */
    ospi_struct->wrap_size = OSPI_DIRECT;                          /* 外部存储器设备不支持回卷读取 */
    ospi_struct->cs_hightime = OSPI_CS_HIGH_TIME_4_CYCLE;          /* 片选高电平时间为4个周期(4*1000/75=53ns),即手册里面的tSHSL参数 */
    ospi_struct->memory_type = OSPI_MICRON_MODE;                   /* 外部设备类型为Micron模式 */
    ospi_struct->delay_hold_cycle = OSPI_DELAY_HOLD_NONE;          /* 不延迟保持 */

    ospi_init(ospi_periph, ospi_struct);                           /* 初始化OSPI参数 */
    
    ospi_enable(ospi_periph);                                      /* 使能OSPI */     
}

/**
 * @brief       OSPI发送命令
 * @param       ins     : 要发送的指令
 * @param       addr    : 发送到的目的地址
 * @param       datalen : 要传输的数据长度
 * @param       mode: 模式,详细位定义如下:
 *   @arg       mode[2:0]:  指令模式; 000,无指令;  001,单线传输指令; 010,双线传输指令; 011,四线传输指令; 100,八线传输指令.
 *   @arg       mode[5:3]:  地址模式; 000,无地址;  001,单线传输地址; 010,双线传输地址; 011,四线传输地址; 100,八线传输地址.
 *   @arg       mode[7:6]:  地址长度; 00,8位地址;   01,16位地址;      10,24位地址;      11,32位地址.
 *   @arg       mode[10:8]: 数据模式; 000,无数据;  001,单线传输数据; 010,双线传输数据; 011,四线传输数据; 100,八线传输数据.
 * @param       dmcycle: 空指令周期数
 * @retval      无
 */
void ospi_send_command(uint32_t ins, uint32_t addr, uint32_t datalen, uint16_t mode, uint8_t dmcycle)
{
    ospi_regular_cmd_struct cmd_struct = {0};                         /* OSPI常规命令结构体 */
 
    cmd_struct.instruction = ins;                                     /* 设置要发送的指令 */
    cmd_struct.address = addr;                                        /* 设置要发送的地址 */
    cmd_struct.dummy_cycles = dmcycle;                                /* 设置空指令周期数 */
    cmd_struct.nbdata = datalen;                                      /* 设置要传输的数据量 */
    
    if(((mode >> 0) & 0x07) == 0)
    cmd_struct.ins_mode = OSPI_INSTRUCTION_NONE;                      /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 1)
    cmd_struct.ins_mode = OSPI_INSTRUCTION_1_LINE;                    /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 2)
    cmd_struct.ins_mode = OSPI_INSTRUCTION_2_LINES;                   /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 3)
    cmd_struct.ins_mode = OSPI_INSTRUCTION_4_LINES;                   /* 指令模式 */    
    else if(((mode >> 0) & 0x07) == 4)
    cmd_struct.ins_mode = OSPI_INSTRUCTION_8_LINES;                   /* 指令模式 */    

    if(((mode >> 3) & 0x07) == 0)
    cmd_struct.addr_mode = OSPI_ADDRESS_NONE;                         /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 1)
    cmd_struct.addr_mode = OSPI_ADDRESS_1_LINE;                       /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 2)
    cmd_struct.addr_mode = OSPI_ADDRESS_2_LINES;                      /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 3)
    cmd_struct.addr_mode = OSPI_ADDRESS_4_LINES;                      /* 地址模式 */    
    else if(((mode >> 3) & 0x07) == 4)
    cmd_struct.addr_mode = OSPI_ADDRESS_8_LINES;                      /* 地址模式 */  

    if(((mode >> 6) & 0x03) == 0)
    cmd_struct.addr_size = OSPI_ADDRESS_8_BITS;                       /* 地址大小 */
    else if(((mode >> 6) & 0x03) == 1)
    cmd_struct.addr_size = OSPI_ADDRESS_16_BITS;                      /* 地址大小 */
    else if(((mode >> 6) & 0x03) == 2)
    cmd_struct.addr_size = OSPI_ADDRESS_24_BITS;                      /* 地址大小 */
    else if(((mode >> 6) & 0x03) == 3) 
    cmd_struct.addr_size = OSPI_ADDRESS_32_BITS;                      /* 地址大小 */    
 
    if(((mode >> 8) & 0x07) == 0)
    cmd_struct.data_mode = OSPI_DATA_NONE;                            /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 1)
    cmd_struct.data_mode = OSPI_DATA_1_LINE;                          /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 2)
    cmd_struct.data_mode = OSPI_DATA_2_LINES;                         /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 3)
    cmd_struct.data_mode = OSPI_DATA_4_LINES;                         /* 数据模式 */    
    else if(((mode >> 8) & 0x07) == 4)
    cmd_struct.data_mode = OSPI_DATA_8_LINES;                         /* 数据模式 */     
 
    cmd_struct.operation_type = OSPI_OPTYPE_COMMON_CFG;               /* 通用配置(间接模式或状态轮询模式下使用)，配置应用于常规寄存器 */
    cmd_struct.ins_size = OSPI_INSTRUCTION_8_BITS;                    /* 设置指令大小为8位指令 */
    cmd_struct.addr_dtr_mode = OSPI_ADDRDTR_MODE_DISABLE;             /* 地址阶段禁用DTR模式 */
    cmd_struct.alter_bytes_mode = OSPI_ALTERNATE_BYTES_NONE;          /* 无交替字节 */
    cmd_struct.data_dtr_mode = OSPI_DADTR_MODE_DISABLE;               /* 数据阶段禁用DTR模式 */

    ospi_command_config(OSPI_INTERFACE, &g_ospi_struct, &cmd_struct); /* 配置OSPI常规命令参数 */    
}













