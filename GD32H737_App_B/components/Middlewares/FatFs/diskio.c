/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Include HAL and SDIO headers for SD card */
#include "stm32f4xx_hal.h"
#include "sdio.h"
#include "rtc.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */


/* Flag for DMA transfer completion */
static volatile uint8_t sd_xfer_complete = 1;

/*--------------------------------------------------------------------------
 * SD card DMA transfer complete callbacks (must be called from HAL)
 *--------------------------------------------------------------------------*/
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
    sd_xfer_complete = 1;
}
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
    sd_xfer_complete = 1;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (BYTE pdrv) {
    DSTATUS stat = 0;

    switch (pdrv) {
        case DEV_FLASH :
            // TODO: Implement FLASH disk status
            // result = FLASH_disk_status();
            // translate the result code here
            stat = STA_NOINIT; // placeholder
            break;

        case DEV_MMC : {
            // Check SD card state
            HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd);
            if (state == HAL_SD_CARD_TRANSFER) {
                stat = 0;                     // Ready
            } else {
                stat = STA_NOINIT;             // Not initialized
            }
            // Optionally check write protection (if applicable)
            // if (sd_is_write_protected) stat |= STA_PROTECT;
            break;
        }

        case DEV_USB :
            // TODO: Implement USB disk status
            // result = USB_disk_status();
            // translate the result code here
            stat = STA_NOINIT; // placeholder
            break;

        default:
            stat = STA_NOINIT;
    }
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (BYTE pdrv) {
    DSTATUS stat = 0;

    switch (pdrv) {
        case DEV_FLASH :
            // TODO: Implement FLASH disk initialization
            // result = FLASH_disk_initialize();
            // translate the result code here
            stat = STA_NOINIT; // placeholder
            break;

        case DEV_MMC : {
            // Initialize SD card via HAL
            if (HAL_SD_Init(&hsd) != HAL_OK) {
                stat = STA_NOINIT;
            } else {
                // Optional: get card info and cache (not required here)
                stat = 0;
            }
            break;
        }

        case DEV_USB :
            // TODO: Implement USB disk initialization
            // result = USB_disk_initialize();
            // translate the result code here
            stat = STA_NOINIT; // placeholder
            break;

        default:
            stat = STA_NOINIT;
    }
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    DRESULT res = RES_OK;

    switch (pdrv) {
        case DEV_FLASH :
            // TODO: Implement FLASH disk read
            // result = FLASH_disk_read(buff, sector, count);
            // translate result to DRESULT
            res = RES_PARERR; // placeholder
            break;

        case DEV_MMC : {
            // Wait for any previous DMA to complete
            while (!sd_xfer_complete) HAL_Delay(1);

            sd_xfer_complete = 0;
            if (HAL_SD_ReadBlocks_DMA(&hsd, buff, (uint32_t)sector, count) != HAL_OK) {
                sd_xfer_complete = 1; // restore flag
                res = RES_ERROR;
                break;
            }

            // Wait for DMA completion
            while (!sd_xfer_complete) HAL_Delay(1);
            res = RES_OK;
            break;
        }

        case DEV_USB :
            // TODO: Implement USB disk read
            // result = USB_disk_read(buff, sector, count);
            // translate result to DRESULT
            res = RES_PARERR; // placeholder
            break;

        default:
            res = RES_PARERR;
    }
    return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0
DRESULT disk_write (BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    DRESULT res = RES_OK;

    switch (pdrv) {
        case DEV_FLASH :
            // TODO: Implement FLASH disk write
            // result = FLASH_disk_write(buff, sector, count);
            // translate result to DRESULT
            res = RES_PARERR; // placeholder
            break;

        case DEV_MMC : {
            // Wait for any previous DMA to complete
            while (!sd_xfer_complete) HAL_Delay(1);

            sd_xfer_complete = 0;
            if (HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t*)buff, (uint32_t)sector, count) != HAL_OK) {
                sd_xfer_complete = 1;
                res = RES_ERROR;
                break;
            }

            // Wait for DMA completion
            while (!sd_xfer_complete) HAL_Delay(1);
            res = RES_OK;
            break;
        }

        case DEV_USB :
            // TODO: Implement USB disk write
            // result = USB_disk_write(buff, sector, count);
            // translate result to DRESULT
            res = RES_PARERR; // placeholder
            break;

        default:
            res = RES_PARERR;
    }
    return res;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff) {
    DRESULT res = RES_OK;

    switch (pdrv) {
        case DEV_FLASH :
            // TODO: Process ioctl for FLASH drive
            res = RES_PARERR; // placeholder
            break;

        case DEV_MMC : {
            HAL_SD_CardInfoTypeDef cardInfo;
            if (HAL_SD_GetCardInfo(&hsd, &cardInfo) != HAL_OK) {
                res = RES_ERROR;
                break;
            }

            switch (cmd) {
                case CTRL_SYNC:
                    // Ensure no ongoing transfer
                    while (!sd_xfer_complete) HAL_Delay(1);
                    res = RES_OK;
                    break;

                case GET_SECTOR_COUNT:
                    *(DWORD*)buff = cardInfo.BlockNbr;
                    break;

                case GET_SECTOR_SIZE:
                    *(WORD*)buff = cardInfo.BlockSize;
                    break;

                case GET_BLOCK_SIZE:
                    *(DWORD*)buff = 1;   // Erase block size in sectors (usually 1)
                    break;

                default:
                    res = RES_PARERR;
            }
            break;
        }

        case DEV_USB :
            // TODO: Process ioctl for USB drive
            res = RES_PARERR; // placeholder
            break;

        default:
            res = RES_PARERR;
    }
    return res;
}



DWORD get_fattime(void) {
    RTC_DateTypeDef sdate;
    RTC_TimeTypeDef stime;

    HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);

    // 按照 FatFs 格式组装 DWORD：
    // bit31:25 = 年-1980, bit24:21 = 月, bit20:16 = 日,
    // bit15:11 = 时, bit10:5 = 分, bit4:0 = 秒/2
    return ((DWORD)(sdate.Year + 2000 - 1980) << 25)
         | ((DWORD)sdate.Month << 21)
         | ((DWORD)sdate.Date << 16)
         | ((DWORD)stime.Hours << 11)
         | ((DWORD)stime.Minutes << 5)
         | ((DWORD)stime.Seconds >> 1);
}
