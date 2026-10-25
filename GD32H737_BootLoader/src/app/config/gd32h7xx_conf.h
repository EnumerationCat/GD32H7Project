/**
  ******************************************************************************
  * @file    gd32h7xx_conf.h
  * @author  GigaDevice Application Team
  * @brief   Configuration file for GD32H7 series.
  *          This file should be copied to the application folder and renamed
  *          to gd32h7xx_conf.h.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */ 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GD32H7xx_CONF_H
#define __GD32H7xx_CONF_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file 
  */
#include "gd32h7xx_adc.h"
#include "gd32h7xx_axiim.h"
#include "gd32h7xx_can.h"
#include "gd32h7xx_cau.h"
#include "gd32h7xx_cmp.h"
#include "gd32h7xx_cpdm.h"
#include "gd32h7xx_crc.h"
#include "gd32h7xx_ctc.h"
#include "gd32h7xx_dac.h"
#include "gd32h7xx_dbg.h"
#include "gd32h7xx_dci.h"
#include "gd32h7xx_dma.h"
#include "gd32h7xx_edout.h"
#include "gd32h7xx_efuse.h"
#include "gd32h7xx_enet.h"
#include "gd32h7xx_exmc.h"
#include "gd32h7xx_exti.h"
#include "gd32h7xx_fac.h"
#include "gd32h7xx_fmc.h"
#include "gd32h7xx_fwdgt.h"
#include "gd32h7xx_gpio.h"
#include "gd32h7xx_hau.h"
#include "gd32h7xx_hpdf.h"
#include "gd32h7xx_hwsem.h"
#include "gd32h7xx_i2c.h"
#include "gd32h7xx_ipa.h"
#include "gd32h7xx_lpdts.h"
#include "gd32h7xx_mdio.h"
#include "gd32h7xx_mdma.h"
#include "gd32h7xx_misc.h"
#include "gd32h7xx_ospi.h"
#include "gd32h7xx_ospim.h"
#include "gd32h7xx_pmu.h"
#include "gd32h7xx_rameccmu.h"
#include "gd32h7xx_rcu.h"
#include "gd32h7xx_rspdif.h"
#include "gd32h7xx_rtc.h"
#include "gd32h7xx_rtdec.h"
#include "gd32h7xx_sai.h"
#include "gd32h7xx_sdio.h"
#include "gd32h7xx_spi.h"
#include "gd32h7xx_syscfg.h"
#include "gd32h7xx_timer.h"
#include "gd32h7xx_tli.h"
#include "gd32h7xx_tmu.h"
#include "gd32h7xx_trigsel.h"
#include "gd32h7xx_trng.h"
#include "gd32h7xx_usart.h"
#include "gd32h7xx_vref.h"
#include "gd32h7xx_wwdgt.h"


/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed. 
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */
#ifdef __cplusplus
}
#endif
#endif /* __GD32H7xx_CONF_H */