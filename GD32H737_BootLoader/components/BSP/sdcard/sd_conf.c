/**
 ****************************************************************************************************
 * @file        sd_conf.c
 * @version     V1.0
 * @brief       SD����ʼ�� ��������
 ****************************************************************************************************
 * @attention   Waiken-Smart ������Զ
 *
 * ʵ��ƽ̨:    GD32H737VMT6Сϵͳ��
 *
 ****************************************************************************************************
 */
 
#include "sd_conf.h"
#include "usart.h"


#define BUSMODE_1BIT    0
#define BUSMODE_4BIT    1


/* ����SDIO������ģʽ, ѡ������ģʽ: BUSMODE_1BIT/BUSMODE_4BIT */
#define SDIO_BUSMODE        BUSMODE_4BIT                          /* ѡ��4λ����ģʽ */

/* ����SDIO�ٶ�ģʽ, ѡ���ٶ�ģʽ: SD_SPEED_DEFAULT/SD_SPEED_HIGH/SD_SPEED_SDR25/SD_SPEED_SDR50/SD_SPEED_SDR104 */
#define SDIO_SPEEDMODE      SD_SPEED_HIGH                         /* ����Ϊ����ģʽ(���ʱ��Ƶ��50M,������ѹ3.3V) */              

/* �������ݴ���ģʽ, ѡ����ģʽ: SD_POLLING_MODE/SD_DMA_MODE */
#define SDIO_DTMODE         SD_POLLING_MODE                       /* ����Ϊ��ѯģʽ */

sd_card_info_struct sd_cardinfo;                                  /* SD����Ϣ */


/**
 * @brief       ��ʼ��SD������ȡSD����Ϣ����������ģʽ�ʹ���ģʽ
 * @param       ��
 * @retval      sd_error_enum
 */
sd_error_enum sdio_sd_init(void)
{
    sd_error_enum status = SD_OK;
    uint32_t cardstate = 0;

    nvic_irq_enable(SDIO0_IRQn, 2, 0);    /* SDIO�ж����ã���ռ���ȼ�2�������ȼ�0 */
  
    status = sd_init();                   /* ��ʼ��SD�� */      
   
    if (SD_OK == status)
    {
        status = sd_card_information_get(&sd_cardinfo);          /* ��ȡ����Ϣ */
    }
    
    if (SD_OK == status)       
    {
        status = sd_card_select_deselect(sd_cardinfo.card_rca);  /* ѡ��SD�� */
    } 
    
    status = sd_cardstatus_get(&cardstate);                      /* ��ȡ��״̬ */
    
    if (cardstate & 0x02000000)                                  /* ����ס�� */                    
    {
        printf("\r\n the card is locked!");
      
        status = sd_lock_unlock(SD_UNLOCK);                      /* ������ */
        
        if (status != SD_OK) 
        {
            return SD_LOCK_UNLOCK_FAILED;                        /* ������ʧ�� */  
        } 
        else 
        {
            printf("\r\n the card is unlocked successfully!");   /* �������ɹ� */  
        }
    }
    
    if ((SD_OK == status) && (!(cardstate & 0x02000000))) 
    {
        /* ��������ģʽ���ٶ�ģʽ */
#if (SDIO_BUSMODE == BUSMODE_4BIT)
        status = sd_bus_mode_config(SDIO_BUSMODE_4BIT, SDIO_SPEEDMODE);  /* 4λSDIO������ģʽ */ 
#else
        status = sd_bus_mode_config(SDIO_BUSMODE_1BIT, SDIO_SPEEDMODE);
#endif
    }

#ifdef USE_18V_SWITCH
    if (SD_OK == status) 
    {
        /* UHS-I Hosts can perform sampling point tuning using tuning command   */
        status = sd_tuning();
    }
#endif /* USE_18V_SWITCH */
    
    if(SD_OK == status)
    {
        /* ���ô��䷽ʽ 
         * ���ʹ�ù�����1.8V�ĸ���ģʽ��ѡ��DMAģʽ 
         */
        status = sd_transfer_mode_config(SDIO_DTMODE);
    }
    
    return status;
}

/**
 * @brief       ͨ�����ڴ�ӡSD�������Ϣ
 * @param       ��
 * @retval      ��
 */
void card_info_get(void)
{
    uint8_t sd_spec, sd_spec3, sd_spec4, sd_security;
    uint32_t block_count, block_size;
    uint16_t temp_ccc;
  
    printf("\r\n Card information:");
    sd_spec = (sd_scr[1] & 0x0F000000) >> 24;
    sd_spec3 = (sd_scr[1] & 0x00008000) >> 15;
    sd_spec4 = (sd_scr[1] & 0x00000400) >> 10; /* SD��SCR�Ĵ�������ȡSD���汾��Ϣ */
  
    if (2 == sd_spec) 
    {
        if (1 == sd_spec3) 
        {
            if (1 == sd_spec4) 
            {
                printf("\r\n## Card version 4.xx ##");
            } 
            else 
            {
                printf("\r\n## Card version 3.0x ##");
            }
        } 
        else 
        {
            printf("\r\n## Card version 2.00 ##");
        }
    } 
    else if (1 == sd_spec) 
    {
        printf("\r\n## Card version 1.10 ##");
    } 
    else if (0 == sd_spec) 
    {
        printf("\r\n## Card version 1.0x ##");
    }

    sd_security = (sd_scr[1] & 0x00700000) >> 20;
    
    if (2 == sd_security) 
    {
        printf("\r\n## security v1.01 ##");
    }
    else if (3 == sd_security)
    {
        printf("\r\n## security v2.00 ##");
    } 
    else if (4 == sd_security) 
    {
        printf("\r\n## security v3.00 ##");
    }

    block_count = (sd_cardinfo.card_csd.c_size + 1) * 1024;
    block_size = 512;
    printf("\r\n## Device size is %dKB ##", sd_card_capacity_get());  /* ��ʾ���� */
    printf("\r\n## Block size is %dB ##", block_size);                /* ��ʾ���С */
    printf("\r\n## Block count is %d ##", block_count);               /* ��ʾ������ */

    if (sd_cardinfo.card_csd.read_bl_partial) 
    {
        printf("\r\n## Partial blocks for read allowed ##");
    }
    
    if (sd_cardinfo.card_csd.write_bl_partial) 
    {
        printf("\r\n## Partial blocks for write allowed ##");
    }
    
    temp_ccc = sd_cardinfo.card_csd.ccc;                              /* SD��������� */
    printf("\r\n## CardCommandClasses is: %x ##", temp_ccc);
    
    if ((SD_CCC_BLOCK_READ & temp_ccc) && (SD_CCC_BLOCK_WRITE & temp_ccc)) 
    {
        printf("\r\n## Block operation supported ##");
    }
    
    if (SD_CCC_ERASE & temp_ccc) 
    {
        printf("\r\n## Erase supported ##");
    }
    
    if (SD_CCC_WRITE_PROTECTION & temp_ccc) 
    {
        printf("\r\n## Write protection supported ##");
    }
    
    if (SD_CCC_LOCK_CARD & temp_ccc) 
    {
        printf("\r\n## Lock unlock supported ##");
    }
    
    if (SD_CCC_APPLICATION_SPECIFIC & temp_ccc) 
    {
        printf("\r\n## Application specific supported ##");
    }
    
    if (SD_CCC_IO_MODE & temp_ccc) 
    {
        printf("\r\n## I/O mode supported ##");
    }
    
    if (SD_CCC_SWITCH & temp_ccc) 
    {
        printf("\r\n## Switch function supported ##");
    }
}

/*!
    \brief      this function handles SDIO0 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SDIO0_IRQHandler(void)
{
    sd_interrupts_process();
}

/**
 * @brief       ��SD��(fatfs����)
 * @param       pbuf  : ���ݻ�����
 * @param       saddr : ������ַ
 * @param       cnt   : ��������
 * @retval      SD_OK, ����;  ����, �������(���sd_error_enum����);
 */
sd_error_enum sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    sd_error_enum status = SD_OK;

    if (cnt == 1)
    {
        status = sd_block_read(pbuf, saddr, 512);              /* ��ȡ����block */
    }
		else 
    {
        status = sd_multiblocks_read(pbuf, saddr, 512, cnt);   /* ��ȡ���block */  
    }

    return status;
}

/**
 * @brief       дSD��(fatfs����)
 * @param       pbuf  : ���ݻ�����
 * @param       saddr : ������ַ
 * @param       cnt   : ��������
 * @retval      SD_OK, ����;  ����, �������(���sd_error_enum����);
 */
sd_error_enum sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    sd_error_enum status = SD_OK;

    if (cnt == 1)
    {
        status = sd_block_write(pbuf, saddr, 512);    	        /* д����block */
    }
		else 
    {
        status = sd_multiblocks_write(pbuf, saddr, 512, cnt);   /* д���block */  
    }

    return status;  
}

/**
 * @brief       ��SD��(usb����)
 * @param       pbuf  : ���ݻ�����
 * @param       saddr : ������ַ
 * @param       cnt   : ��������
 * @retval      SD_OK, ����;  ����, �������(���sd_error_enum����);
 */
sd_error_enum sd_read_udisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    sd_error_enum status = SD_OK;
    uint32_t lsaddr = saddr;

    lsaddr >>= 9;                /* GD USB�ⲻ����������ַ��SD�� */

    if (cnt == 1)
    {
        status = sd_block_read(pbuf, lsaddr, 512);              /* ��ȡ����block */
    }
		else 
    {
        status = sd_multiblocks_read(pbuf, lsaddr, 512, cnt);   /* ��ȡ���block */  
    }

    return status;
}

/**
 * @brief       дSD��(usb����)
 * @param       pbuf  : ���ݻ�����
 * @param       saddr : ������ַ
 * @param       cnt   : ��������
 * @retval      SD_OK, ����;  ����, �������(���sd_error_enum����);
 */
sd_error_enum sd_write_udisk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    sd_error_enum status = SD_OK;
    uint32_t lsaddr = saddr;

    lsaddr >>= 9;                /* GD USB�ⲻ����������ַдSD�� */

    if (cnt == 1)
    { 
        status = sd_block_write(pbuf, lsaddr, 512);    	         /* д����block */
    }
		else 
    {
        status = sd_multiblocks_write(pbuf, lsaddr, 512, cnt);   /* д���block */  
    }

    return status;  
}

