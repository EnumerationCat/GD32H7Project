#include "startup_gd32h737vmt6.h"
#include "cmsis_gcc.h"



/* This symbol comes from the linker scirpt. */
extern uint32_t __INITIAL_SP;

/* This symbol comes from cmsis_gcc_m.h, wrapped _start() function. */
extern __NO_RETURN void __PROGRAM_START();



__NO_RETURN void Reset_Handler();
void Default_Handler(void);

/* -------------------------- 异常处理函数声明 -------------------------- */
void NMI_Handler()                      __attribute__((weak));
void HardFault_Handler()                __attribute__((weak));
void MemManage_Handler()                __attribute__((weak));
void BusFault_Handler()                 __attribute__((weak));
void UsageFault_Handler()               __attribute__((weak));
void SVC_Handler()                      __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler()                 __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler()                   __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler()                  __attribute__((weak, alias("Default_Handler")));

/* -------------------------- 外部中断处理函数声明 -------------------------- */
void WWDGT_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void AVD_LVD_OVD_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void TAMPER_STAMP_LXTAL_IRQHandler()    __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void FMC_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void RCU_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel0_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel1_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel2_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel3_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel4_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel5_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel6_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void ADC0_1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void EXTI5_9_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER0_BRK_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER0_UP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void TIMER0_TRG_CMT_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void TIMER0_Channel_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void TIMER1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void TIMER2_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void TIMER3_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void I2C0_EV_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void I2C0_ER_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void SPI0_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void USART0_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void EXTI10_15_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void TIMER7_BRK_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER7_UP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void TIMER7_TRG_CMT_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void TIMER7_Channel_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void DMA0_Channel7_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void EXMC_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SDIO0_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void TIMER4_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void UART3_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void TIMER5_DAC_UDR_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void TIMER6_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel0_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void ENET0_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void ENET0_WKUP_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void USART5_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void USBHS0_EP1_OUT_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void USBHS0_EP1_IN_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void USBHS0_WKUP_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void USBHS0_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void DCI_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void CAU_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void HAU_TRNG_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void UART6_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void UART7_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SPI4_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SPI5_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SAI0_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void TLI_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void TLI_ER_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void IPA_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void SAI1_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void OSPI0_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void RSPDIF_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void DMAMUX_OVR_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void HPDF_INT0_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void HPDF_INT1_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void HPDF_INT2_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void HPDF_INT3_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void SAI2_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void TIMER14_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER15_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER16_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void MDIO_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void MDMA_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void SDIO1_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void HWSEM_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void ADC2_IRQHandler()                  __attribute__((weak, alias("Default_Handler")));
void CMP0_1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void CTC_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void RAMECCMU_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void OSPI1_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void RTDEC0_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void RTDEC1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void FAC_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void TMU_IRQHandler()                   __attribute__((weak, alias("Default_Handler")));
void TIMER22_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER23_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER30_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER31_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER40_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER41_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER42_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER43_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER44_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER50_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void TIMER51_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
void USBHS1_EP1_OUT_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void USBHS1_EP1_IN_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
void USBHS1_WKUP_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void USBHS1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
void ENET1_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void ENET1_WKUP_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void CAN0_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void CAN0_Message_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
void CAN0_Busoff_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void CAN0_Error_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void CAN0_FastError_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void CAN0_TEC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void CAN0_REC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void CAN1_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void CAN1_Message_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
void CAN1_Busoff_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void CAN1_Error_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void CAN1_FastError_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void CAN1_TEC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void CAN1_REC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void CAN2_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void CAN2_Message_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
void CAN2_Busoff_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void CAN2_Error_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void CAN2_FastError_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
void CAN2_TEC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void CAN2_REC_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
void EFUSE_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void I2C0_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void I2C1_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void I2C2_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void I2C3_WKUP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
void LPDTS_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
void LPDTS_WKUP_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER0_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER7_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER1_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER2_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER3_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER4_DEC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
void TIMER22_DEC_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void TIMER23_DEC_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void TIMER30_DEC_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
void TIMER31_DEC_IRQHandler()           __attribute__((weak, alias("Default_Handler")));


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

// -------------------------- 中断向量表 --------------------------
const VectorType __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
    // 异常向量（前16项）
    (VectorType)(&__INITIAL_SP),          // 0: 栈顶指针
    Reset_Handler,                        // 1: 复位处理函数
    NMI_Handler,                          // 2: NMI处理函数
    HardFault_Handler,                    // 3: 硬故障处理函数
    MemManage_Handler,                    // 4: 存储管理故障
    BusFault_Handler,                     // 5: 总线故障
    UsageFault_Handler,                   // 6: 使用故障
    0,                                    // 7: 保留
    0,                                    // 8: 保留
    0,                                    // 9: 保留
    0,                                    // 10: 保留
    SVC_Handler,                          // 11: 系统调用
    DebugMon_Handler,                     // 12: 调试监控
    0,                                    // 13: 保留
    PendSV_Handler,                       // 14: 挂起请求
    SysTick_Handler,                      // 15: 系统滴答定时器

    // 外部中断（从16开始，与汇编向量表完全对齐）
    WWDGT_IRQHandler,                     // 16: 窗口看门狗
    AVD_LVD_OVD_IRQHandler,               // 17: 电压检测
    TAMPER_STAMP_LXTAL_IRQHandler,        // 18: RTC篡改/时间戳/LXTAL时钟安全
    RTC_WKUP_IRQHandler,                  // 19: RTC唤醒
    FMC_IRQHandler,                       // 20: FMC全局中断
    RCU_IRQHandler,                       // 21: RCU全局中断
    EXTI0_IRQHandler,                     // 22: EXTI0
    EXTI1_IRQHandler,                     // 23: EXTI1
    EXTI2_IRQHandler,                     // 24: EXTI2
    EXTI3_IRQHandler,                     // 25: EXTI3
    EXTI4_IRQHandler,                     // 26: EXTI4
    DMA0_Channel0_IRQHandler,             // 27: DMA0通道0
    DMA0_Channel1_IRQHandler,             // 28: DMA0通道1
    DMA0_Channel2_IRQHandler,             // 29: DMA0通道2
    DMA0_Channel3_IRQHandler,             // 30: DMA0通道3
    DMA0_Channel4_IRQHandler,             // 31: DMA0通道4
    DMA0_Channel5_IRQHandler,             // 32: DMA0通道5
    DMA0_Channel6_IRQHandler,             // 33: DMA0通道6
    ADC0_1_IRQHandler,                    // 34: ADC0/ADC1
    0,                                    // 35: 保留
    0,                                    // 36: 保留
    0,                                    // 37: 保留
    0,                                    // 38: 保留
    EXTI5_9_IRQHandler,                   // 39: EXTI5-9
    TIMER0_BRK_IRQHandler,                // 40: TIMER0刹车
    TIMER0_UP_IRQHandler,                 // 41: TIMER0更新
    TIMER0_TRG_CMT_IRQHandler,            // 42: TIMER0触发/换向
    TIMER0_Channel_IRQHandler,            // 43: TIMER0捕获比较
    TIMER1_IRQHandler,                    // 44: TIMER1
    TIMER2_IRQHandler,                    // 45: TIMER2
    TIMER3_IRQHandler,                    // 46: TIMER3
    I2C0_EV_IRQHandler,                   // 47: I2C0事件
    I2C0_ER_IRQHandler,                   // 48: I2C0错误
    I2C1_EV_IRQHandler,                   // 49: I2C1事件
    I2C1_ER_IRQHandler,                   // 50: I2C1错误
    SPI0_IRQHandler,                      // 51: SPI0
    SPI1_IRQHandler,                      // 52: SPI1
    USART0_IRQHandler,                    // 53: USART0
    USART1_IRQHandler,                    // 54: USART1
    USART2_IRQHandler,                    // 55: USART2
    EXTI10_15_IRQHandler,                 // 56: EXTI10-15
    RTC_Alarm_IRQHandler,                 // 57: RTC闹钟
    0,                                    // 58: 保留
    TIMER7_BRK_IRQHandler,                // 59: TIMER7刹车
    TIMER7_UP_IRQHandler,                 // 60: TIMER7更新
    TIMER7_TRG_CMT_IRQHandler,            // 61: TIMER7触发/换向
    TIMER7_Channel_IRQHandler,            // 62: TIMER7捕获比较
    DMA0_Channel7_IRQHandler,             // 63: DMA0通道7
    EXMC_IRQHandler,                      // 64: EXMC
    SDIO0_IRQHandler,                     // 65: SDIO0
    TIMER4_IRQHandler,                    // 66: TIMER4
    SPI2_IRQHandler,                      // 67: SPI2
    UART3_IRQHandler,                     // 68: UART3
    UART4_IRQHandler,                     // 69: UART4
    TIMER5_DAC_UDR_IRQHandler,            // 70: TIMER5/DAC欠载
    TIMER6_IRQHandler,                    // 71: TIMER6
    DMA1_Channel0_IRQHandler,             // 72: DMA1通道0
    DMA1_Channel1_IRQHandler,             // 73: DMA1通道1
    DMA1_Channel2_IRQHandler,             // 74: DMA1通道2
    DMA1_Channel3_IRQHandler,             // 75: DMA1通道3
    DMA1_Channel4_IRQHandler,             // 76: DMA1通道4
    ENET0_IRQHandler,                     // 77: ENET0
    ENET0_WKUP_IRQHandler,                // 78: ENET0唤醒
    0,                                    // 79: 保留
    0,                                    // 80: 保留
    0,                                    // 81: 保留
    0,                                    // 82: 保留
    0,                                    // 83: 保留
    DMA1_Channel5_IRQHandler,             // 84: DMA1通道5
    DMA1_Channel6_IRQHandler,             // 85: DMA1通道6
    DMA1_Channel7_IRQHandler,             // 86: DMA1通道7
    USART5_IRQHandler,                    // 87: USART5
    I2C2_EV_IRQHandler,                   // 88: I2C2事件
    I2C2_ER_IRQHandler,                   // 89: I2C2错误
    USBHS0_EP1_OUT_IRQHandler,            // 90: USBHS0端点1输出
    USBHS0_EP1_IN_IRQHandler,             // 91: USBHS0端点1输入
    USBHS0_WKUP_IRQHandler,               // 92: USBHS0唤醒
    USBHS0_IRQHandler,                    // 93: USBHS0
    DCI_IRQHandler,                       // 94: DCI
    CAU_IRQHandler,                       // 95: CAU
    HAU_TRNG_IRQHandler,                  // 96: HAU/TRNG
    FPU_IRQHandler,                       // 97: FPU
    UART6_IRQHandler,                     // 98: UART6
    UART7_IRQHandler,                     // 99: UART7
    SPI3_IRQHandler,                      // 100: SPI3
    SPI4_IRQHandler,                      // 101: SPI4
    SPI5_IRQHandler,                      // 102: SPI5
    SAI0_IRQHandler,                      // 103: SAI0
    TLI_IRQHandler,                       // 104: TLI
    TLI_ER_IRQHandler,                    // 105: TLI错误
    IPA_IRQHandler,                       // 106: IPA
    SAI1_IRQHandler,                      // 107: SAI1
    OSPI0_IRQHandler,                     // 108: OSPI0
    0,                                    // 109: 保留
    0,                                    // 110: 保留
    I2C3_EV_IRQHandler,                   // 111: I2C3事件
    I2C3_ER_IRQHandler,                   // 112: I2C3错误
    RSPDIF_IRQHandler,                    // 113: RSPDIF
    0,                                    // 114: 保留
    0,                                    // 115: 保留
    0,                                    // 116: 保留
    0,                                    // 117: 保留
    DMAMUX_OVR_IRQHandler,                // 118: DMAMUX溢出
    0,                                    // 119: 保留
    0,                                    // 120: 保留
    0,                                    // 121: 保留
    0,                                    // 122: 保留
    0,                                    // 123: 保留
    0,                                    // 124: 保留
    0,                                    // 125: 保留
    HPDF_INT0_IRQHandler,                 // 126: HPDF全局中断0
    HPDF_INT1_IRQHandler,                 // 127: HPDF全局中断1
    HPDF_INT2_IRQHandler,                 // 128: HPDF全局中断2
    HPDF_INT3_IRQHandler,                 // 129: HPDF全局中断3
    SAI2_IRQHandler,                      // 130: SAI2
    0,                                    // 131: 保留
    TIMER14_IRQHandler,                   // 132: TIMER14
    TIMER15_IRQHandler,                   // 133: TIMER15
    TIMER16_IRQHandler,                   // 134: TIMER16
    0,                                    // 135: 保留
    MDIO_IRQHandler,                      // 136: MDIO
    0,                                    // 137: 保留
    MDMA_IRQHandler,                      // 138: MDMA
    0,                                    // 139: 保留
    SDIO1_IRQHandler,                     // 140: SDIO1
    HWSEM_IRQHandler,                     // 141: HWSEM
    0,                                    // 142: 保留
    ADC2_IRQHandler,                      // 143: ADC2
    0,                                    // 144: 保留
    0,                                    // 145: 保留
    0,                                    // 146: 保留
    0,                                    // 147: 保留
    0,                                    // 148: 保留
    0,                                    // 149: 保留
    0,                                    // 150: 保留
    0,                                    // 151: 保留
    0,                                    // 152: 保留
    CMP0_1_IRQHandler,                    // 153: CMP0/CMP1
    0,                                    // 154: 保留
    0,                                    // 155: 保留
    0,                                    // 156: 保留
    0,                                    // 157: 保留
    0,                                    // 158: 保留
    0,                                    // 159: 保留
    CTC_IRQHandler,                       // 160: 时钟恢复系统
    RAMECCMU_IRQHandler,                  // 161: RAMECCMU
    0,                                    // 162: 保留
    0,                                    // 163: 保留
    0,                                    // 164: 保留
    0,                                    // 165: 保留
    OSPI1_IRQHandler,                     // 166: OSPI1
    RTDEC0_IRQHandler,                    // 167: RTDEC0
    RTDEC1_IRQHandler,                    // 168: RTDEC1
    FAC_IRQHandler,                       // 169: FAC
    TMU_IRQHandler,                       // 170: TMU
    0,                                    // 171: 保留
    0,                                    // 172: 保留
    0,                                    // 173: 保留
    0,                                    // 174: 保留
    0,                                    // 175: 保留
    0,                                    // 176: 保留
    TIMER22_IRQHandler,                   // 177: TIMER22
    TIMER23_IRQHandler,                   // 178: TIMER23
    TIMER30_IRQHandler,                   // 179: TIMER30
    TIMER31_IRQHandler,                   // 180: TIMER31
    TIMER40_IRQHandler,                   // 181: TIMER40
    TIMER41_IRQHandler,                   // 182: TIMER41
    TIMER42_IRQHandler,                   // 183: TIMER42
    TIMER43_IRQHandler,                   // 184: TIMER43
    TIMER44_IRQHandler,                   // 185: TIMER44
    TIMER50_IRQHandler,                   // 186: TIMER50
    TIMER51_IRQHandler,                   // 187: TIMER51
    USBHS1_EP1_OUT_IRQHandler,            // 188: USBHS1端点1输出
    USBHS1_EP1_IN_IRQHandler,             // 189: USBHS1端点1输入
    USBHS1_WKUP_IRQHandler,               // 190: USBHS1唤醒
    USBHS1_IRQHandler,                    // 191: USBHS1
    ENET1_IRQHandler,                     // 192: ENET1
    ENET1_WKUP_IRQHandler,                // 193: ENET1唤醒
    0,                                    // 194: 保留
    CAN0_WKUP_IRQHandler,                 // 195: CAN0唤醒
    CAN0_Message_IRQHandler,              // 196: CAN0消息缓冲区
    CAN0_Busoff_IRQHandler,               // 197: CAN0总线关闭
    CAN0_Error_IRQHandler,                // 198: CAN0错误
    CAN0_FastError_IRQHandler,            // 199: CAN0快速传输错误
    CAN0_TEC_IRQHandler,                  // 200: CAN0发送警告
    CAN0_REC_IRQHandler,                  // 201: CAN0接收警告
    CAN1_WKUP_IRQHandler,                 // 202: CAN1唤醒
    CAN1_Message_IRQHandler,              // 203: CAN1消息缓冲区
    CAN1_Busoff_IRQHandler,               // 204: CAN1总线关闭
    CAN1_Error_IRQHandler,                // 205: CAN1错误
    CAN1_FastError_IRQHandler,            // 206: CAN1快速传输错误
    CAN1_TEC_IRQHandler,                  // 207: CAN1发送警告
    CAN1_REC_IRQHandler,                  // 208: CAN1接收警告
    CAN2_WKUP_IRQHandler,                 // 209: CAN2唤醒
    CAN2_Message_IRQHandler,              // 210: CAN2消息缓冲区
    CAN2_Busoff_IRQHandler,               // 211: CAN2总线关闭
    CAN2_Error_IRQHandler,                // 212: CAN2错误
    CAN2_FastError_IRQHandler,            // 213: CAN2快速传输错误
    CAN2_TEC_IRQHandler,                  // 214: CAN2发送警告
    CAN2_REC_IRQHandler,                  // 215: CAN2接收警告
    EFUSE_IRQHandler,                     // 216: EFUSE
    I2C0_WKUP_IRQHandler,                 // 217: I2C0唤醒
    I2C1_WKUP_IRQHandler,                 // 218: I2C1唤醒
    I2C2_WKUP_IRQHandler,                 // 219: I2C2唤醒
    I2C3_WKUP_IRQHandler,                 // 220: I2C3唤醒
    LPDTS_IRQHandler,                     // 221: LPDTS
    LPDTS_WKUP_IRQHandler,                // 222: LPDTS唤醒
    TIMER0_DEC_IRQHandler,                // 223: TIMER0 DEC
    TIMER7_DEC_IRQHandler,                // 224: TIMER7 DEC
    TIMER1_DEC_IRQHandler,                // 225: TIMER1 DEC
    TIMER2_DEC_IRQHandler,                // 226: TIMER2 DEC
    TIMER3_DEC_IRQHandler,                // 227: TIMER3 DEC
    TIMER4_DEC_IRQHandler,                // 228: TIMER4 DEC
    TIMER22_DEC_IRQHandler,               // 229: TIMER22 DEC
    TIMER23_DEC_IRQHandler,               // 230: TIMER23 DEC
    TIMER30_DEC_IRQHandler,               // 231: TIMER30 DEC
    TIMER31_DEC_IRQHandler                // 232: TIMER31 DEC
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif



extern void SystemInit();
/* Forward declaration of main function */



__NO_RETURN void Reset_Handler()
{
    SystemInit();// CMSIS系统初始化（时钟配置等）
    __PROGRAM_START();
}




void Default_Handler()
{
    for ( ; ; ) {}
}

void NMI_Handler()
{
    for ( ; ; ) {}
}

void HardFault_Handler()
{
   // 定义变量存储崩溃现场
    uint32_t fault_pc;
    __ASM volatile ("mov %0, pc" : "=r"(fault_pc));
    // 断点打在这里，调试器查看 fault_pc = 出错代码地址
    while(1)
    {
        __NOP();
    }
}

void MemManage_Handler()
{
    for ( ; ; ) {}
}

void BusFault_Handler()
{
    for ( ; ; ) {}
}

void UsageFault_Handler()
{
    for ( ; ; ) {}
}




