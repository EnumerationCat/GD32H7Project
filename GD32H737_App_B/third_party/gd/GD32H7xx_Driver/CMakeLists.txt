# GD32H7xx_Driver
# CMakeLists for GD32H7xx Standard Peripheral Library
# 驱动源码同时提供给 App(app) 与 Bootloader(bootloader)

foreach(t ${FIRMWARE_TARGETS})
target_compile_definitions(${t} PRIVATE
    USE_FULL_LL_DRIVER
)

target_include_directories(${t} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Inc"
    
)

target_sources(${t} PRIVATE
    # ==================== 核心基础外设  ====================
    # 时钟/系统配置
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_rcu.c"        # RCC_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_syscfg.c"     # 系统配置（配合EXTI/GPIO等核心外设）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_misc.c"       # CORTEX_MODULE_ENABLED
   
    # 通用IO/中断
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_gpio.c"       # GPIO_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_exti.c"       # EXTI_MODULE_ENABLED
    # 电源/复位/看门狗
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_pmu.c"        # PWR_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_fwdgt.c"      # IWDG_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_wwdgt.c"      # WWDG_MODULE_ENABLED
    # 存储/校验
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_fmc.c"        # FLASH_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_crc.c"        # CRC_MODULE_ENABLED

    # ==================== 常用功能外设 (按需保留，与conf配置对齐) ====================
    # 模拟外设
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_adc.c"        # ADC_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_dac.c"        # DAC_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_vref.c"       # 模拟参考电压（配合ADC/DAC）
    # 通信外设
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_i2c.c"        # I2C_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_spi.c"        # SPI_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_usart.c"      # USART_MODULE_ENABLED
    # 定时器/DMA
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_timer.c"      # TIM_MODULE_ENABLED
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_dma.c"        # DMA_MODULE_ENABLED
    # 实时时钟
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_rtc.c"        # RTC_MODULE_ENABLED

    # ==================== 未启用/不常用外设 (注释掉，按需开启) ====================
    # 1. 总线/通信类（conf中未启用或小众）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_can.c"        # CAN_MODULE_ENABLED（若需CAN则开启）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_sdio.c"       # SD_MODULE_ENABLED（若需SD卡则开启）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_ospi.c"       # QSPI_MODULE_ENABLED（若需QSPI则开启）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_ospim.c"      # QSPI配套驱动
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_sai.c"        # SAI_MODULE_ENABLED（音频接口）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_enet.c"      
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_mdio.c"       # 以太网MDIO配套

    # 2. 安全/加密类
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_hau.c"        # HASH_MODULE_ENABLED（哈希算法）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_hau_sha_md5.c"# SHA/MD5算法配套
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cau.c"        # CRYP_MODULE_ENABLED（加密）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cau_aes.c"    # AES加密配套
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cau_des.c"    # DES加密配套
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cau_tdes.c"   # 3DES加密配套
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_trng.c"       # 真随机数发生器

    # 3. 特殊功能类
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_exmc.c"       # 外部存储控制器
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_dci.c"        # DCMI_MODULE_ENABLED（摄像头接口）
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_dbg.c"        # 调试模块
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_efuse.c"      # 熔丝配置
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_hwsem.c"      # 硬件信号量
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_mdma.c"       # 多核DMA
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_rspdif.c"     # SPDIF音频
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_tmu.c"        # 温度监测单元
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cmp.c"        # 比较器
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_ctc.c"        # 电容触摸控制
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_trigsel.c"    # 触发选择
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_ipa.c"        # IPA协议
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_lpdts.c"      # 低功耗定时器
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_rameccmu.c"   # RAM ECC校验
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_rtdec.c"      # 实时解码
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_tli.c"        # TLI接口
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_cpdm.c"       # CPDM模块
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_edout.c"      # EDOUT模块
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_fac.c"        # FAC模块
    "${CMAKE_CURRENT_LIST_DIR}/Src/gd32h7xx_hpdf.c"       # HPDF模块
)
endforeach()