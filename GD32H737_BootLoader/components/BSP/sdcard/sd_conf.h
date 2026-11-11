/**
 ****************************************************************************************************
 * @file        sd_conf.h
 * @version     V1.0
 * @brief       SD����ʼ�� ��������
 ****************************************************************************************************
 * @attention   Waiken-Smart ������Զ
 *
 * ʵ��ƽ̨:    GD32H737VMT6Сϵͳ��
 *
 ****************************************************************************************************
 */
 
#ifndef __SD_CONF_H
#define __SD_CONF_H

#include "sys.h"
#include "sdio_sdcard.h"


extern sd_card_info_struct sd_cardinfo;   /* SD����Ϣ */

/******************************************************************************************/

sd_error_enum sdio_sd_init(void);         /* ��ʼ��SD�� */
void card_info_get(void);                 /* ��ȡSD�������Ϣ */

sd_error_enum sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);    /* ��SD��(fatfs����) */
sd_error_enum sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);   /* дSD��(fatfs����) */
sd_error_enum sd_read_udisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);   /* ��SD��(usb����) */
sd_error_enum sd_write_udisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);  /* дSD��(usb����) */

#endif







